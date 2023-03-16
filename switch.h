typedef struct forwardingTable {
    int valid[99];
    int destination[99];
    int port[99];
} forwardingTable;

void switchMain();

forwardingTable * initializeTable();
forwardingTable * resetTable(forwardingTable *currTable);

int addLink();
int receivePacket();
int sendPacket();
