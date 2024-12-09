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
Runs the critical section for a given node in the network.
*/
int criticalSection(char *message[]){
    

    // print message from node

    return 1;
}


/*

*/
int run_network() {



    return 1;
}



/*
Removes an existing message queue.
*/
int remove_message_queue(int key) {
    if ((msgctl(key, IPC_RMID, NULL)) == -1) {
        perror("Message queue removal failed: ");
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
    struct msgbuf msg; // message queue buffer struct var
    int i = 'a'; 
    key_t key = ftok(".", i); // key for message queue
    int msg_id;
    
    if ((msg_id = msgget(key, msgflg)) == -1) { // error for mssgget
        perror("Message queue creation error:");
        return 1;
    }
    printf("Message queue created successfully!\n");
    return msg_id;
}


int main(int argc, char *argv[]) {


    
    // init message queue
    int msg_id = create_message_queue();



    // run program's main functions


    // remove message queue
    remove_message_queue(msg_id);

    exit(0);
}
