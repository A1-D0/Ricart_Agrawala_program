/*
Description: Runs the server for the Ricart-Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
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
    return msg_id;
}

/*
Runs the server to print node messages.
*/
int run_server(int msg_id, struct msgbuf *msg_buf) {
    while (TRUE) {
        if (msgrcv(msg_id, msg_buf, sizeof(msg_buf->msg_text), 3, 0) > -1) { 
            if (msg_buf->msg_type == 3) { // verify it's a node message
                printf("%s \n", msg_buf->msg_text); // print message sent from node
                memset(msg_buf->msg_text, 0, BUFFER_SIZE); // clear buffer
            } else fprintf(stderr, "Unexpected message type in server process: %zu\n", msg_buf->msg_type);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    fflush(stdout); // immediately flush output
    
    // init message queue
    int msg_id; // message queue id
    if ((msg_id = create_message_queue()) < 0) exit(1);
    struct msgbuf msg_buf; // message queue buffer struct var

    // run program's main functions
    run_server(msg_id, &msg_buf);

    // remove message queue
    if (remove_message_queue(msg_id) < 0) exit(2);

    exit(0);
}
