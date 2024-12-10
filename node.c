/*
Description: Runs a node that is integrated into and participates in the network for the Ricart Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
Depedencies: 
*/

/* NOTES
1. The nodes will print to a shared memory, which, for the chosen node, will then be sent to the server to print to the terminal.
2. 
*/

#include <iso646.h>
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
int send_message(int msg_id, struct msgbuf *msg_buf) {

    msg_buf->msg_type = 1;
    strcpy(msg_buf->msg_text, "this is line 1!");
    printf("Message to be sent: %s\n", msg_buf->msg_text);

    if (msgsnd(msg_id, msg_buf, BUFFER_SIZE + 1, IPC_NOWAIT) < 0) {
        perror("Message could not be sent to server.\n");
        return 1;
    }
    printf("Message '%s' sent successfully!\n", msg_buf->msg_text);
    printf("String length is: %zu\n", strlen(msg_buf->msg_text));

    return 0;
}

/*
Gets an existing message queue from the kernel.
*/
int get_message_queue() {
    // init message parameters
    int msgq_id; // message queue id var
    int msgflg = IPC_CREAT | 0666; // message flag
    int i = 'a'; 
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

    // init message queue
    struct msgbuf msg_buf; // message queue buffer struct var
    int msg_id;
    if ((msg_id = get_message_queue()) == 1) {
        exit(1);
    }

    // send message to server
    send_message(msg_id, &msg_buf);



    exit(0);
}

// example output: ### START OUTPUT FOR NODE 4 ###
//4 this is line 1
//4this is line 2
//-- END OUTPUT FOR NODE 4 --
//### START OUTPUT FOR NODE 3 ###
//3 first lie of output
//HACKER Ja ja ja...
//3 hello world
//-- END OUTPUT FOR NODE 3 --
//......