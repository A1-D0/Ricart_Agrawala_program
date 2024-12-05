/*
Description: 
Author: Osvaldo Hernandez-Segura
Depedencies: 
*/

/* NOTES
1. The nodes will print to a shared memory, which, for the chosen node, will then be sent to the server to print to the terminal.
2. The server will manage the critical section for print.
*/



# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <semaphore.h>


/*
Runs the network for the Ricart Agrawala algorithm.
*/
int runNetwork() {



    return 1;
}








int main(int argc, char *argv[]) {

    runNetwork();

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