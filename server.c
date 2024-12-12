/*
Description: Runs the network and critical section for the Ricart-Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
*/

/* NOTES
1. The chosen node will print one line at a time to a shared memory, which will then be sent to the server to print, within the CS, to the terminal.
2. The server will manage the critical section for print.
3. To remove all semaphores, use ipcrm -a
4. To view all semaphores: ipcs -s
5. To view all shared memory: ipcs -m
6. 
*/

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/ipc.h> // ipc key generation
# include <sys/msg.h> // message queue functions

# define BUFFER_SIZE 50

enum{FALSE, TRUE};

/*
Message queue buffer.
 */
struct msgbuf {
    long msg_type; // type of message
    char msg_text[BUFFER_SIZE]; // message buffer for text of message
};


/*
Removes an existing message queue.
*/
int remove_message_queue(int msg_id) {
    if ((msgctl(msg_id, IPC_RMID, NULL)) == -1) {
        perror("Message queue removal failed!\n");
        return 1;
    }
    return 0;
}

/*
Creates a message queue for the server.
*/
int create_message_queue() {
    // init message parameters
    int msgq_id; // message queue id var
    int msgflg = IPC_CREAT | 0666; // message flag
    int i = 'z'; 
    key_t key = ftok(".", i); // key for message queue
    int msg_id; // message queue id
    
    if ((msg_id = msgget(key, msgflg)) == -1) { // error for mssgget
        perror("Message queue creation error!\n");
        return 1;
    }
    printf("Message queue %d created successfully!\n", msg_id);
    return msg_id;
}

/*
Runs the server to print node messages.
*/
int run_server(int msg_id, struct msgbuf *msg_buf) {
    printf("Running server...\n");
    while (TRUE) {
        // printf("In loop...\n"); 
        int validation;
        validation = msgrcv(msg_id, msg_buf, sizeof(msg_buf->msg_text), 3, 0); // accept type 3 (note 4th parameter)
        // printf("Validation is: %d\n", validation);
        if (validation > -1) { 
            // printf("Message receive by node or hacker is not empty.\n");
            if (msg_buf->msg_type == 3) { // verify it's a node message
                printf("%s \n", msg_buf->msg_text); // print message sent from node
                memset(msg_buf->msg_text, 0, BUFFER_SIZE); // clear buffer
            } else {
                fprintf(stderr, "Unexpected message type in server process: %zu\n", msg_buf->msg_type);
            }
        } else {
            // fprintf(stderr, "Error receiving message in server process: %zu\n", msg_buf->msg_type);
        }
    }
    // printf("Message '%s' recieved successfully!\n", msg_buf->msg_text);
    return 0;
}

int main(int argc, char *argv[]) {
    fflush(stdout); // immediately flush output
    
    // init message queue
    int msg_id; // message queue id
    if ((msg_id = create_message_queue()) < 0) { // error
        exit(1);
    }
    struct msgbuf msg_buf; // message queue buffer struct var

    // run program's main functions
    run_server(msg_id, &msg_buf);

    // remove message queue
    if (remove_message_queue(msg_id) < 0) { // error
        exit(2);
    }

    exit(0);
}
