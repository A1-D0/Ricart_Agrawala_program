/*
Description: 
Author: Osvaldo Hernandez-Segura
Depedencies: 
*/

/* NOTES
1. The nodes will print to a shared memory, which, for the chosen node, will then be sent to the server to print to the terminal.
2. The hacker will not communicate with the other nodes; it will act on its own and send a print whenever it is summoned.
*/

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <semaphore.h>

typedef struct hacker {
    int N = 0; // number of nodes
    int request_number; // nodes sequence number
    int highest_request_number; // highest request number seen
    int outstanding_reply; // # of outstanding replies
};



int main(int argc, char *argv[]) {

    int n = runNetwork();

    exit(n);
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