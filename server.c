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

typedef struct hacker {
    int N = 0; // number of nodes
    int request_number; // nodes sequence number
    int highest_request_number; // highest request number seen
    int outstanding_reply; // # of outstanding replies
};



int runNetwork() {

    return 1;
}








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