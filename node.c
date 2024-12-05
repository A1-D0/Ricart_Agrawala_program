/*
Description: 
Author: Osvaldo Hernandez-Segura
Depedencies: 
*/

/* NOTES
1. The nodes will print to a shared memory, which, for the chosen node, will then be sent to the server to print to the terminal.
2. 
*/

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <semaphore.h>

typedef struct node {
    int N = 0; // number of nodes
    int request_number; // nodes sequence number
    int highest_request_number; // highest request number seen
    int outstanding_reply; // # of outstanding replies
    int request_CS; // true when node requests critical section
    int reply_deferred[N]; // reply_deferred[i] is true when node defers reply to node i
//    semaphore mutex; // for mutual exclusion to shared variables
//    semaphore wait_sem; // used to wait for all requests
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

}



int main(int argc, char *argv[]) {



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