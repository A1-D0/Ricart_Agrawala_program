/*
Description: Runs the network and critical section for the Ricart Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
Depedencies: 
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






/*
Message queue buffer.
 */
struct msgbuf {
    long msg_type; // type of message
    char msg_text[BUFFER_SIZE]; // message buffer for text of message
};




















/*
Runs the critical section to print a given node's messages.
*/
int criticalSection(char *message[]){
    

    // print message from node

    return 1;
}


/*
Runs the server to print node messages.
*/
int run_server(int msg_id, struct msgbuf *msg_buf) {
    printf("Running server...\n");
    while (1) {
        // receive message from message queue
        if (msgrcv(msg_id, msg_buf, BUFFER_SIZE + 1, 1, 0) < 0) {
            perror("Error is receiving message from queue!\n");
            return 1;
        } else if (strlen(msg_buf->msg_text) > 0) {
            break;
        }
    }
    printf("Message '%s' recieved successfully!\n", msg_buf->msg_text);
    return 0;
}

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

int main(int argc, char *argv[]) {
    
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
