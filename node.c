/*
Description: Runs a node that is integrated into and participates in the network for the Ricart Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
*/

# include <iso646.h>
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

int ME; // this node's number
int outstanding_reply; // number of outstanding replies 
int request_number; // nodes sequence number 

enum{FALSE, TRUE};

// /*
// Semaphore buffer.
// */
// struct sembuf {
//     unsigned short sem_num; // semaphore number (index)
//     short int sem_op; // semaphore operation (i.e., increment or decrement)
//     short int sem_flag; // operations flag
// };

/*
Shared memory for shared variables among all existing nodes in the network.
*/
struct shared_variables {
    int N; // number of nodes
    int highest_request_number; // highest request number seen 
    int request_CS; // true (1) when node requests critical section; otherwise, false (0)
    int reply_deferred[MAX_NODES]; // reply_deferred[i] is true (1) when node defers reply to node i; otherwise, false (0)
    int mutex; // binary semaphore for mutual exclusion to shared variables 
    int wait_sem; // binary semaphore used to wait for all requests 
};


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







// /*
// Prints the message to the shared memory.

// Arguments: 
//     prints (int): number of prints.
// */
// int print_message(unsigned int prints) {
//     int idx = 0;
//     for (; idx < prints; idx++) {
//         printf("This is line %d", prints); // note: change the fdes number to that corresponding to the shared memory
//     }
//     return 0;
// }

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
Removes shared memory for last node.
*/
int remove_shared_memory(int shm_id, struct shared_variables *shm_vars) {
    if (shm_vars->N == 1) { // last node condition
        if ((shmctl(shm_id, IPC_RMID, NULL)) < 0) {
            return 1;
        }
    }
    return 0;
}

/*
Detach this node from shared memory.
*/
int detach_shared_memory(struct shared_variables *shm_vars) {
    if (shmdt(shm_vars) < 0) {
        perror("Failed to detach from shared memory!\n");
        return 1;
    }
    return 0;
}

/*
Attach to shared memory.
*/
int attach_shared_memory(int shm_id, struct shared_variables **shm_vars) {
    *shm_vars = (struct shared_variables *)shmat(shm_id, NULL, 0);
    if (*shm_vars == (void *)-1) {
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
int access_shared_variables(int shm_id, struct shared_variables *shm_vars) {
    if (attach_shared_memory(shm_id, &shm_vars)) return 1;
    (shm_vars->N)++;
    (shm_vars->highest_request_number)++;
    shm_vars->request_CS = 0;
    // shm_vars->mutex = 0;
    // shm_vars->wait_sem = 0;
    printf("Node %d accessed shared variables successfully!\n", ME);
    return 0;
}

/*
Runs the error-free network for node ipc.
*/
void run_node_network(struct shared_variables *shm_vars) {
    shm_vars->mutex = create_semaphore(); // get semaphore for this node
    if (ME == 1) { // conditional check: ensure only first node initialize the shared (memory) variables
        set_semaphore_value(shm_vars->mutex, 1); // set binary semaphore mutex value to 1 (unlocked)
        // if ((shm_vars->reply_deferred = malloc(MAX_NODES * sizeof(int))) < 0) { // allocate memory for reply_deferred pointer to int array
        //     perror("Failed to allocate memory for reply_deferred pointer!\n");
        //     exit(10);
        // }
        for (int idx = 0; idx < MAX_NODES; idx++) shm_vars->reply_deferred[idx] = FALSE;
        shm_vars->N = 1;
        shm_vars->highest_request_number = 1;
        printf("Shared variables initialized by node 1.\n");
    } else {
        printf("Node %d attached to existing shared memory.\n", ME);
    }
    printf("Semaphore %d for mutex attained successfully!\n", shm_vars->mutex);

    // get shared (memory) variables
    int shm_id = get_shared_memory();

    // access shared (memory) variables via CS
    P(shm_vars->mutex);

    // CS
    printf("Node %d in CS, yay!!!\n", ME);
    access_shared_variables(shm_id, shm_vars);

    V(shm_vars->mutex);

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
    struct shared_variables shm_vars; 
    set_my_node_number(atoi(argv[1]));
    // int shm_id = get_shared_memory();


    // init message queue
    struct msgbuf msg_buf; // message queue buffer struct var
    int msg_id;
    if ((msg_id = get_message_queue())  == 1) exit(1);


    // run error-free node network
    run_node_network(&shm_vars);


    // send message to server iff this node receives all node REPLYs
    // if (send_message_to_server(msg_id, &msg_buf)  == 1) exit(4);


    // remove ipcs
    // remove shared memory
    // if (remove_shared_memory(shm_id, &shm_vars) == 1) exit(5);

    // remove allocate memory for reply_deferred pointer; ensure last exsting node does this

    exit(0);
}


/*
TO DO NEXT: 
Implement Ricart-A algorithm for nodes.
*/