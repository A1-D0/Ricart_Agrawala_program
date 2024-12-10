/*
Description: Runs a node that is integrated into and participates in the network for the Ricart Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
Depedencies: 
*/

/* NOTES
1. The nodes will print to a shared memory, which, for the chosen node, will then be sent to the server to print to the terminal.
2. 
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

enum{FALSE, TRUE};

/*
Shared memory for shared variables among all existing nodes in the network.
*/
struct shared_variables {
    int me;// this node's number
    int N; // number of nodes
    int request_number; // nodes sequence number 
    int highest_request_number; // highest request number seen 
    int outstanding_reply; // number of outstanding replies 
    int request_CS; // true (1) when node requests critical section; otherwise, false (0)
    int *reply_deferred; // reply_deferred[i] is true (1) when node defers reply to node i; otherwise, false (0)
    int mutex; // semaphore for mutual exclusion to shared variables 
    int wait_sem; // semaphore used to wait for all requests 
};

/*
Semaphore buffer.
*/
struct semaphore {
    ushort semval; // semaphore value, nonnegative 
    short sempid; // pid of last operation 
    ushort semncnt; // number of awaiting semval > cval 
    ushort semzcnt; // number of awaiting semval = 0
};

/*
Message queue buffer.
 */
struct msgbuf {
    long msg_type; // type of message
    char msg_text[BUFFER_SIZE]; // message buffer for text of message
};

/*
Set node number given in CLI to this node.
*/
void set_my_node_number(int node_number, struct shared_variables *shm_vars) {
    shm_vars->me = node_number;
}

// void set_reply_deferred(struct shared_variables *shm_vars) {
//     int reply_def[shm_vars->N];
//     shm_vars->reply_deferred = reply_def;
// }







/*
Get semaphore resource (P operation).
*/
int P(int sem_id) {




    return 0;
}

/*
Release semaphore resource (V operation).
*/
int V(int sem_id) {





    return 0;
}



// /*
// Send REPLY to node i from this node.
// */
// void send(int reply, int i) {

// }

// /*
// REPLY to incoming node; receive REQUEST messages.
// k is the sequence number being requested;
// i is the node making the request.
// */
// int RESPOND(int k, int i, struct shared_variables *shm_vars) {

//     int defer_it;

//     if (k > shm_vars->highest_request_number) {
//         shm_vars->highest_request_number = k;
//     }
//     P(shm_vars->mutex); // enter CS for ipc

//     defer_it = (shm_vars->request_CS) && ((k > shm_vars->request_number) || (k == shm_vars->request_number && i > shm_vars->me));

//     V(shm_vars->mutex); // exit CS for ipc

//     if (defer_it) { // defer_it is true (1) if we have a priority
//         shm_vars->reply_deferred[i] = TRUE;
//     } else {
//         send(REPLY, i); // send REPLY to node i
//     }


//     return 0;
// }

// void receive_REPLY(struct shared_variables *shm_vars) {
//     shm_vars->outstanding_reply -= 1;
//     V(shm_vars->wait_sem);
// }

// /*
// REQUEST to another node.
// */
// int REQUEST() {

//     return 0;
// }






/*
Prints the message to the shared memory.

Arguments: 
    prints (int): number of prints.
*/
int printMessage(unsigned int prints) {
    int idx = 0;
    for (; idx < prints; idx++) {
        printf("This is line %d", prints); // note: change the fdes number to that corresponding to the shared memory
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
Removes shared memory for last node.
*/
int remove_shared_memory(int shm_id) {
    if ((shmctl(shm_id, IPC_RMID, NULL)) < 0) {
        return 1;
    }
    return 0;
}

/*
Gets shared memory for first node.
*/
int create_shared_memory(int node_number) {
    if (node_number != 1) {
        printf("This node is not number 1; cannot create shared memory!\n");
        return 1;
    }
    // conditional check: ensure only first node created shared memory
    int shm_id; 
    int i = 'S';
    key_t key = ftok(".", i);
    int msgflg = IPC_CREAT | 0666;

    if ((shm_id = shmget(key, 1000, msgflg)) < 0) {
        perror("Failed to create shared memory!\n");
        return 2;
    }
    printf("Shared memory %d created successfully!\n", shm_id);
    return shm_id;
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
    int i = 'a'; // may need to change declaration to be unqiue per node
    key_t key = ftok(".", i);
    int msgflg = IPC_CREAT | 0666;
    if ((sem_id = semget(key, 1, msgflg)) < 0) { // error
        perror("Failed to create semaphore for node!\n");
        return 1;
    }
    printf("Sempaphore %d created successfully!\n", sem_id);
    return 0;
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Need two arguments: ./node i, where i is the node number!\n");
        exit(-1);
    }

    struct shared_variables shm_vars; // init shared varibles struct
    set_my_node_number(atoi(argv[1]), &shm_vars);
    // printf("My node number is: %d\n", me);
    int shm_id = create_shared_memory(shm_vars.me); // only applies to node 1


    // init message queue
    struct msgbuf msg_buf; // message queue buffer struct var
    int msg_id;
    if ((msg_id = get_message_queue())  == 1) exit(1);

    // // create semaphore
    // struct semaphore sem;
    // int sem_id;
    // if ((sem_id = create_semaphore()) == 1) {
    //     exit(2);
    // }





    // send message to server iff this node receives all node REPLYs
    if (send_message_to_server(msg_id, &msg_buf)  == 1) exit(3);




    // // remove semaphore created
    // if (remove_semaphore(sem_id) == 1) {
    //     exit(4);
    // }

    // remove shared memory
    if (remove_shared_memory(shm_id) == 1) exit(4);


    exit(0);
}
