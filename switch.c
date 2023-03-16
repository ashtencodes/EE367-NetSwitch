#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "main.h"
#include "packet.h"
#include "net.h"
#include "host.h"
#include "switch.h"

forwardingTable * initializeTable(){
    forwardingTable *newTable = (struct forwardingTable *) malloc(sizeof(struct forwardingTable));
    return newTable;
}

forwardingTable * resetTable(forwardingTable *currTable){
    if(currTable){
        for(int i = 0; i < 100; i++){
            currTable->valid[i] = -1;
            currTable->destination[i] = -1;
            currTable->port[i] = -1;
        }
    }
    return currTable;
}

void switchMain(){
    while(1){
        //printf("Switch Table Running lesgooo\n");
    }
}
