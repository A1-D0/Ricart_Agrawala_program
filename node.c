/*
Description: Runs a node that is integrated into and participates in the network for the Ricart-Agrawala algorithm.
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
    msg_buf->msg_type = 3; // type 3 is for node to server message
    memset(msg_buf->msg_text, 0, BUFFER_SIZE); // clear buffer
    sprintf(msg_buf->msg_text, "########## START OUTPUT FOR NODE %d ##########", ME);
    
    printf("Sending message to server...\n");

    for (int n = 0; n < 6; n++) { // send body lines to server 5 times
        if (msgsnd(msg_id, msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) { // first iter, send header
            perror("Message could not be sent to server.\n");
            return 1;
        }
        memset(msg_buf->msg_text, 0, BUFFER_SIZE); // clear buffer
        sprintf(msg_buf->msg_text, "%d this is line %d", ME, (n + 1));
    }
    memset(msg_buf->msg_text, 0, BUFFER_SIZE); // clear buffer
    sprintf(msg_buf->msg_text, "---------- END OUTPUT FOR NODE %d ----------", ME);
    // printf("msg_text is: %s\n", msg_buf->msg_text);
    if (msgsnd(msg_id, msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) { // send footer
        perror("Message could not be sent to server.\n");
        return 1;
    }
    memset(msg_buf->msg_text, 0, BUFFER_SIZE); // clear buffer
    return 0;
}



/*
Send reply to node i via message queue.
*/
void send_rep(int reply, int i, int msg_id) {
    struct msgbuf msg_buf; // message queue buffer struct var
    msg_buf.msg_type = 2; // type 2 for REPLY
    sprintf(msg_buf.msg_text, "REPLY %d", i); // write to msg_text
    if (msgsnd(msg_id, &msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) perror("send_rep failed!\n");
    // printf("Message from node %d sent!\n", ME);
}

/*
Send REQUEST to node i.
*/
void send_req(int request, int me, int i, int request_number, int msg_id) {
    struct msgbuf msg_buf; // message queue buffer struct var
    msg_buf.msg_type = 1; // type 1 for REQUEST
    sprintf(msg_buf.msg_text, "REQUEST %d %d %d", ME, i, request_number); // write to msg_text
    if (msgsnd(msg_id, &msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) perror("send_req failed!\n");
    printf("Message %s from node %d sent!\n", msg_buf.msg_text, ME);
}

/*
Process which receives request (k,j) messages.
k is the sequence number being requested; 
i is the node making the request.
*/
void receive_request(int k, int i, int msg_id) {
    int defer_it;

    if (k > highest_request_number) highest_request_number = k;

    P(mutex);

    defer_it = (request_CS) && ((k > request_number) || (k == request_number && i > ME));

    V(mutex);

    if (defer_it) reply_deferred[i] = TRUE;
    else send_rep(REPLY, i, msg_id); // send REPLY to node i via message queue
}

/*
Process which invokes mutual exclusion for this node.
*/
void send_request(int msg_id) {

    P(mutex);
    request_CS = TRUE;
    request_number = highest_request_number++;
    V(mutex);

    outstanding_reply = N - 1;

    int REQUEST = 0; // may need to change to string

    for (int i = 0; i <= N; i++) if (i != ME) send_req(REQUEST, ME, i, request_number, msg_id); // send REQUEST to node i

    while(outstanding_reply != 0); // busy wait

    P(wait_sem);

    // CRITICAL SECTION; send message to server
    printf("In CS...\n");
    struct msgbuf msg_buf;
    send_message_to_server(msg_id, &msg_buf);

    request_CS = FALSE;

    for (int i = 1; i <= N; i++) {
        if (reply_deferred[i]) {
            reply_deferred[i] = FALSE;
            send_rep(REPLY, i, msg_id); // send REPLY to node i via message queue
        }
    }
}

/*
Receive REPLY messages;
prcess which receives reply messages.
*/
void receive_reply() {
    outstanding_reply--;
    V(wait_sem);
}

/*
Enhanced message parsing for REQUEST messages.
*/
void receive_request_message(int msg_id) {
    struct msgbuf msg;
    char msg_type[BUFFER_SIZE];
    int k, i;

    // continuously listen for messages
    while (TRUE) {
        // receive messages of type 1 (REQUEST messages)
        if (msgrcv(msg_id, &msg, BUFFER_SIZE, 1, 0) > 0) {
            // parse the message to extract message type and parameters
            if (sscanf(msg.msg_text, "%s %d %d", msg_type, &i, &k) == 3) {
                // verify it's a REQUEST message
                if (strcmp(msg_type, "REQUEST") == 0) {
                    // call existing receive_request logic with parsed parameters
                    // printf("%s received!\n", msg.msg_text);
                    receive_request(k, i, msg_id);
                } else {
                    fprintf(stderr, "Unexpected message type in request process: %s\n", msg_type);
                }
            } else {
                fprintf(stderr, "Malformed REQUEST message: %s\n", msg.msg_text);
            }
        } else {
            perror("Error receiving REQUEST message\n");
        }
    }
}

/*
Enhanced message parsing for REPLY messages.
*/
void receive_reply_message(int msg_id) {
    struct msgbuf msg;
    char msg_type[BUFFER_SIZE];
    int sender;

    // continuously listen for messages
    while (TRUE) {
        // receive messages of type 2 (REPLY messages)
        if (msgrcv(msg_id, &msg, BUFFER_SIZE, 2, 0) > 0) {
            // parse the message to extract message type and sender
            if (sscanf(msg.msg_text, "%s %d", msg_type, &sender) == 2) {
                // verify it's a REPLY message
                if (strcmp(msg_type, "REPLY") == 0) {
                    // call existing receive_reply logic
                    receive_reply();
                } else {
                    fprintf(stderr, "Unexpected message type in reply process: %s\n", msg_type);
                }
            } else {
                fprintf(stderr, "Malformed REPLY message: %s\n", msg.msg_text);
            }
        } else {
            perror("Error receiving REPLY message");
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
    int i = ME; // create an independent shared memory for each node i
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
    int i = 's' + rand();
    key_t key = ftok(".", i);
    int msgflg = IPC_CREAT | 0666;
    if ((sem_id = semget(key, 1, msgflg)) < 0) { // error
        perror("Failed to create semaphore for node!\n");
        return 1;
    }
    // printf("Sempaphore %d created successfully!\n", sem_id);
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
    printf("Node %d child %d accessed shared variables successfully!\n", ME, getpid());
    return 0;
}

/*
Runs the error-free network for node ipc.
*/
void run_node_network() {
    mutex = create_semaphore(); // get mutex semaphore for this node
    wait_sem = create_semaphore(); // get wait_sem semaphore for this node

    int msg_id;
    if ((msg_id = get_message_queue())  == 1) exit(1); // get message queue (to share among all nodes)
    
    // if (ME == 1) { // conditional check: ensure only first node initialize the shared (memory) variables

        if (semctl(mutex, 0, SETVAL, 1)) { // set mutex to initial value of 1
            perror("Failed to set value to mutex semaphore!\n");
        }

        if (semctl(wait_sem, 0, SETVAL, 1)) { // set wait_sem to initial value of 1
            perror("Failed to set value to wait_sem semaphore!\n");
        }

        // printf("Initial mutex value: %d\n", semctl(mutex, 0, GETVAL));


        for (int idx = 0; idx < MAX_NODES; idx++) reply_deferred[idx] = FALSE;
        N = 1;
        highest_request_number = 1;
    //     printf("Shared variables initialized by node 1.\n");
    // } else {
    //     printf("Node %d attached to existing shared memory.\n", ME);
    // }
    printf("Semaphore %d for mutex attained successfully!\n", mutex);
    printf("Semaphore %d for wait_sem attained successfully!\n", wait_sem);

    // get shared (memory) variables
    int shm_id = get_shared_memory();


    // fork three child processes
    int pid_receive_request;
    int pid_send_request;
    int pid_receive_reply;

    if ((pid_receive_request = fork()) == 0) { // in child process
        printf("Forking first child...\n");
        receive_request_message(msg_id);
        exit(0); // exit child process
    }

    if ((pid_send_request = fork()) == 0) { // in child process
        printf("Forking second child...\n");
        while (TRUE) {
            send_request(msg_id);
            sleep(1);
        }
        exit(0); // exit child process
    }

    if ((pid_receive_reply = fork()) == 0) { // in child process
        printf("Forking third child...\n");
        receive_reply_message(msg_id);
        exit(0); // exit child process
    }

    // in parent process; wait for child processes to terminate
    wait(NULL);
    wait(NULL);
    wait(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Need two arguments: ./node i, where i is the node number!\n");
        exit(-1);
    }

    // set node number to ME
    ME = atoi(argv[1]);

    // run node network
    run_node_network();

    exit(0);
}

/*
TO DO NEXT:
Send messages to server from node in CS.
*/