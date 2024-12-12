/*
Description: Hacker node which sends a message to server when summoned and is not part of the Ricart-Agrawala algorithm.
Author: Osvaldo Hernandez-Segura
*/

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/ipc.h> // ipc key generation
# include <sys/msg.h> // message queue functions
# include <sys/types.h> 

# define BUFFER_SIZE 50

/*
Message queue buffer.
 */
struct msgbuf {
    long msg_type; // type of message
    char msg_text[BUFFER_SIZE]; // message buffer for text of message
};

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
    int msg_id = get_message_queue();
    struct msgbuf msg_buf;
    msg_buf.msg_type = 3; // type 3 is for node to server message
    sprintf(msg_buf.msg_text, "HACKER Ja ja ja...\n");
    if (msgsnd(msg_id, &msg_buf, BUFFER_SIZE, IPC_NOWAIT) < 0) perror("Message could not be sent to server.\n");
    printf("Message %s sent to server!\n", msg_buf.msg_text);
    exit(0);
}
