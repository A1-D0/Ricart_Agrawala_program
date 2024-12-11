/*
Description: Runs a node that is integrated into and participates in the network for the Ricart Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
*/

// # include <iso646.h> // may not need this
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/ipc.h> // ipc key generation
# include <sys/msg.h> // message queue functions
# include <sys/types.h> 
# include <sys/sem.h> // semaphores
# include <sys/shm.h> // shared memory

# define BUFFER_SIZE 50
# define MAX_NODES 10
# define REPLY 0


enum{FALSE, TRUE};

int ME; // this node's number
int outstanding_reply; // number of outstanding replies 
int request_number; // nodes sequence number 
int N; // number of nodes
int highest_request_number; // highest request number seen 
int request_CS; // true (1) when node requests critical section; otherwise, false (0)
int reply_deferred[MAX_NODES]; // reply_deferred[i] is true (1) when node defers reply to node i; otherwise, false (0)
int mutex; // binary semaphore for mutual exclusion to shared variables 
int wait_sem; // binary semaphore used to wait for all requests 

/*
Message queue buffer.
 */
struct msgbuf {
    long msg_type; // type of message
    char msg_text[BUFFER_SIZE]; // message buffer for text of message
};

/*
Set node number, ME, given in CLI to this node.
*/
void set_my_node_number(int node_number) {
    ME = node_number;
}








/*
Get semaphore resource (P operation).
Decrements the semaphore by one; blocks the process requesting access if semaphore value is less than one (locked).
*/
int P(int sem_id) {
    struct sembuf sem_op;
    sem_op.sem_num = 0; // semaphore index
    sem_op.sem_op = -1; // decrement operation
    sem_op.sem_flg = 0; // no flags
    if (semop(sem_id, &sem_op, 1)) {
        perror("P operation failed!\n");
        return 1;
    }
    return 0;
}

/*
Release semaphore resource (V operation).
Increments the semaphore by one; wakes up a blocked process that previously requested access.
*/
int V(int sem_id) {
    struct sembuf sem_op;
    sem_op.sem_num = 0; // semaphore index
    sem_op.sem_op = 1; // increment operation
    sem_op.sem_flg = 0; // no flags
    if (semop(sem_id, &sem_op, 1)) {
        perror("V operation failed!\n");
        return 1;
    }
    return 0;
}



/*
Send message from node to server.
*/
int send_message_to_server(int msg_id, struct msgbuf *msg_buf) {
    msg_buf->msg_type = 1;
    strcpy(msg_buf->msg_text, "this is line 1!");
    printf("Message to be sent: %s\n", msg_buf->msg_text);

    if (msgsnd(msg_id, msg_buf, BUFFER_SIZE + 1, IPC_NOWAIT) < 0) {
        perror("Message could not be sent to server.\n");
        return 1;
    }
    printf("Message '%s' sent successfully!\n", msg_buf->msg_text);
    // printf("String length is: %zu\n", strlen(msg_buf->msg_text));
    return 0;
}



/*
Send reply to node i via message queue.
*/
void send_rep(int reply, int i, int msg_id) {
    struct msgbuf msg_buf; // message queue buffer struct var
    sprintf(msg_buf.msg_text, "REPLY %d", i); // write to msg_text
    if (msgsnd(msg_id, &msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) perror("send_rep failed!\n");
    printf("Message from node %d sent!\n", ME);
}

/*
Send REQUEST to node i.
*/
void send_req(int request, int me, int request_number, int i, int msg_id) {
    struct msgbuf msg_buf; // message queue buffer struct var
    sprintf(msg_buf.msg_text, "REQUEST %d %d %d", i, ME, request_number); // write to msg_text
    if (msgsnd(msg_id, &msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) perror("send_req failed!\n");
    printf("Message from node %d sent!\n", ME);
}

/*
Receive REPLY messages.
*/
void receive_reply() {
    outstanding_reply--;
    V(wait_sem);
}


/* THREE PROCESSES--MAKE SURE TO FORK THEM */

void receive_request(int k, int i, int msg_id) {
    int defer_it;

    if (k > highest_request_number) highest_request_number = k;

    P(mutex);

    defer_it = (request_CS) && ((k > request_number) || (k == request_number && i > ME));

    V(mutex);

    if (defer_it) reply_deferred[i] = TRUE;
    else send_rep(REPLY, i, msg_id); // send REPLY to node i via message queue
}

void send_request(int msg_id) {

    P(mutex);
    request_CS = TRUE;
    request_number = highest_request_number++;
    V(mutex);

    outstanding_reply = N - 1;

    int REQUEST = 0; // may need to change to string

    for (int i = 0; i <= N; i++) if (i != N) send_req(REQUEST, ME, request_number, i, msg_id); // send REQUEST to node i

    while(outstanding_reply != 0); // busy wait

    P(wait_sem);

    // CRITICAL SECTION

    request_CS = FALSE;

    for (int i = 1; i <= N; i++) {
        if (reply_deferred[i]) {
            reply_deferred[i] = FALSE;
            send_rep(REPLY, i, msg_id); // send REPLY to node i via message queue
        }
    }
}


















/*
Removes shared memory for last node.
*/
int remove_shared_memory(int shm_id) {
    if (N == 1) { // last node condition
        if ((shmctl(shm_id, IPC_RMID, NULL)) < 0) {
            return 1;
        }
    }
    return 0;
}

/*
Detach this node from shared memory.
*/
int detach_shared_memory(int *shm_id) {
    if (shmdt(shm_id) < 0) {
        perror("Failed to detach from shared memory!\n");
        return 1;
    }
    return 0;
}

/*
Attach to shared memory.
*/
int attach_shared_memory(int shm_id) {
    if (shmat(shm_id, NULL, 0) < 0) {
        perror("Failed to attach to shared memory!\n");
        return 1;
    }
    return 0;
}

/*
Creates shared memory for first node; for all other nodes, gets an existing shared memory.
*/
int get_shared_memory() {
    int shm_id; 
    int i = 'm';
    key_t key = ftok(".", i);
    int msgflg = IPC_CREAT | 0666;
    if ((shm_id = shmget(key, 1000, msgflg)) < 0) {
        perror("Failed to create shared memory!\n");
        return 1;
    }
    printf("Shared memory %d created successfully for node %d!\n", shm_id, ME);
    return shm_id;
}

/*
Sets a binary semaphore's value.
*/
void set_semaphore_value(int sem_id, int value) {
    if (semctl(sem_id, 0, SETVAL, value)) {
        perror("Failed to set value to semaphore!\n");
    }
    printf("Initial mutex value: %d\n", semctl(sem_id, 0, GETVAL));
}

/*
Removes a given semaphore.
*/
int remove_semaphore(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) < 0) {
        perror("Failed to remove semaphore!\n");
        return 1;
    }    
    return 0;
}

/*
Creates a semaphore.
*/
int create_semaphore() {
    int sem_id; 
    int i = 's';
    key_t key = ftok(".", i);
    int msgflg = IPC_CREAT | 0666;
    if ((sem_id = semget(key, 1, msgflg)) < 0) { // error
        perror("Failed to create semaphore for node!\n");
        return 1;
    }
    printf("Sempaphore %d created successfully!\n", sem_id);
    return sem_id;
}

/*
Gets an existing message queue from the kernel.
*/
int get_message_queue() {
    // init message parameters
    int msgq_id; // message queue id var
    int msgflg = IPC_CREAT | 0666; // message flag
    int i = 'z'; 
    key_t key = ftok(".", i); // key for message queue
    int msg_id;
    if ((msg_id = msgget(key, msgflg)) < 0) { // error for mssgget
        perror("Message queue error:");
        return 1;
    }
    printf("Message queue %d attained successfully!\n", msg_id);
    return msg_id;
}

/*
Accesses the shared variables stored in the shared memory (which is accessed by all nodes in the network):
N, request number, highest_request_number, 
outstanding_reply, request_CS, reply_deferred[N], 
mutex, and wait_sem.
*/
int access_shared_variables(int shm_id) {
    if (attach_shared_memory(shm_id)) return 1;
    N++;
    highest_request_number++;
    request_CS = 0;
    printf("Node %d accessed shared variables successfully!\n", ME);
    return 0;
}

/*
Runs the error-free network for node ipc.
*/
void run_node_network() {
    mutex = create_semaphore(); // get semaphore for this node
    
    int msg_id;
    if ((msg_id = get_message_queue())  == 1) exit(1); // get message queue (to share among all nodes)
    
    if (ME == 1) { // conditional check: ensure only first node initialize the shared (memory) variables
        set_semaphore_value(mutex, 1); // set binary semaphore mutex value to 1 (unlocked)
        for (int idx = 0; idx < MAX_NODES; idx++) reply_deferred[idx] = FALSE;
        N = 1;
        highest_request_number = 1;
        printf("Shared variables initialized by node 1.\n");
    } else {
        printf("Node %d attached to existing shared memory.\n", ME);
    }
    printf("Semaphore %d for mutex attained successfully!\n", mutex);

    // get shared (memory) variables
    int shm_id = get_shared_memory();

    // access shared (memory) variables via CS
    P(mutex);

    // CS
    printf("Node %d in CS, yay!!!\n", ME);
    access_shared_variables(shm_id);
    printf("Node %d doing work...\n", ME);
    sleep(3); // doing work...

    V(mutex);

    // while (1) { // communicate among other nodes
    //     break;
    // }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Need two arguments: ./node i, where i is the node number!\n");
        exit(-1);
    }

    // init shared varibles struct
    ME = atoi(argv[1]);

    // fork three processes--one for each process (Cf. research paper)

    // run error-free node network
    run_node_network();


    // send message to server iff this node receives all node REPLYs
    // if (send_message_to_server(msg_id, &msg_buf)  == 1) exit(4);


    // remove ipcs
    // remove shared memory
    // if (remove_shared_memory(shm_id, &shm_vars) == 1) exit(5);

    // remove allocate memory for reply_deferred pointer; ensure last exsting node does this

    exit(0);
}