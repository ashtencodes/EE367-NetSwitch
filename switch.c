 /*
  * switch.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "net.h"
#include "man.h"
#include "host.h"
#include "packet.h"
#include "switch.h"

#define TENMILLISEC 10000   /* 10 millisecond sleep */
#define MAX_PORT_TABLE_LENGTH 100

struct packet *new_packet;

// Helper function used to see if something exist within the table and returns at what index
int check_switch_port_forwarding_table(struct switch_port_forwarding table [], int Value)
{
    for (int i = 0; i < MAX_PORT_TABLE_LENGTH; i++)
    {
        if (table[i].valid == Valid && table[i].dst == Value)
        {
            return table[i].port;
        }
    }
    return -1;
}

int assign_entry_in_table(struct switch_port_forwarding table [], char dst, int src_port, enum localPortTreeValue localPortTree)
{
    for (int i = 0; i < MAX_PORT_TABLE_LENGTH; i++)
    {
        if (table[i].valid == NotValid)
        {
            table[i].valid = Valid;
            table[i].dst = dst;
            table[i].port = src_port;
			table[i].localPortTree = localPortTree;
            return i;
        }

    }

	return -1; 
}

int port_to_index(struct switch_port_forwarding table [], int port)
{
	for (int i = 0; i < MAX_PORT_TABLE_LENGTH; i++)
	{
		if (table[i].valid == Valid && table[i].port == port)
		{
			return i;
		}
	}
	return -1;
}

enum localPortTreeValue valid_port_in_table(struct switch_port_forwarding table [], int port)
{
	for (int i = 0; i < MAX_PORT_TABLE_LENGTH; i++)
	{
		if (table[i].valid == Valid && table[i].port == port)
		{
			return table[i].localPortTree;
		}
	}
	return YES;
}

void switch_main(int switch_id)
{

/* State */
struct net_port *node_port_list;
struct net_port **node_port;  // Array of pointers to node ports
int node_port_num;            // Number of node ports

int i, k, n;
int root_node_id = switch_id;

struct packet *in_packet; /* Incoming packet */

struct net_port *p;
struct host_job *new_job;
struct host_job *new_job2;

struct job_queue job_q;

struct switch_port_forwarding MAC_Address_Table[MAX_PORT_TABLE_LENGTH];


/*
 * Initialize Table 
 * Sets the entries as not valid until assigned
 */
for (int i=0; i<MAX_PORT_TABLE_LENGTH; i++){
	MAC_Address_Table[i].valid = NotValid;
}

/*
 * Create an array node_port[ ] to store the network link ports
 * at the switch.  The number of ports is node_port_num
 */
node_port_list = net_get_port_list(switch_id);

	/*  Count the number of network link ports */
node_port_num = 0;
for (p=node_port_list; p!=NULL; p=p->next) {
	node_port_num++;
}
	/* Create memory space for the array */
node_port = (struct net_port **)
	malloc(node_port_num*sizeof(struct net_port *));

	/* Load ports into the array */
p = node_port_list;

for (k = 0; k < node_port_num; k++) {
	node_port[k] = p;
	p = p->next;
}

/* Initialize the job queue */
job_q_init(&job_q);

int localRootID = switch_id;
int localRootDist = 0;
int localParent = -1;

int macAddressIndex;
int destination_port;
int val;

char packet_contents[3];

while(1) {

	for (k = 0; k < node_port_num; k++) { /* Scan all ports */

		in_packet = (struct packet *) malloc(sizeof(struct packet));
		n = packet_recv(node_port[k], in_packet);

		if (n > 0) {

			new_job = (struct host_job *)
				malloc(sizeof(struct host_job));
			new_job->in_port_index = k;
			new_job->packet = in_packet;

			switch(in_packet->type) {

				case (char) PKT_SWITCH_CONTROL: 
					new_job->type = JOB_CONTROL_RECV;
					job_q_add(&job_q, new_job);
					break;
					
				default:
					//printf("switch %d\ngot into default\n count jobs %d \n", switch_id, job_q_num(&job_q));
					job_q_init(&job_q);
					job_q_add(&job_q, new_job);
					break;
			}

		} else {
			free(in_packet);
		}
	}

	/*
 	 * Execute one job in the job queue
 	 */

	if (job_q_num(&job_q) > 0) {

		/* Get a new job from the job queue */
		new_job = job_q_remove(&job_q);
		//printf("%d\n", new_job->type);
		switch(new_job->type) {

			/* Send packets on all ports */	
			case JOB_CONTROL_RECV: //Code to handle control packets

					//printf("payload[2] = %c\n", new_job->packet->payload[2]);
				if (new_job->packet->payload[2] == 'S') {

					 //Local port tree array: tree that represents the MST. edges are 1

					if (new_job->packet->payload[0] < localRootID) {

						localRootID = new_job->packet->payload[0];
						localParent = new_job->packet->src;
						localRootDist = new_job->packet->payload[1] + 1;
						//printf("Switch: %d\n src %d\n localRootID %d \n localParent %d\n local root dest %d\n in port index %d\n", switch_id, new_job->packet->src, localRootID, localParent, localRootDist, new_job->in_port_index);
					} 
					else if (new_job->packet->payload[0] == localRootID) {
						if(new_job->packet->payload[1] + 1 < localRootDist) {
							//printf("Switch: %d\n src %d\n %d < %d\n", switch_id, new_job->packet->src, new_job->packet->payload[1]+1, localRootDist);
							localRootDist = new_job->packet->payload[1] + 1;
							localParent = new_job->packet->src;
							//printf("Switch: %d\n localRootID %d \n localParent %d\n local root dest %d\n in port index %d\n", switch_id, localRootID, localParent, localRootDist, new_job->in_port_index);
						}
					}

				}
				
				if (new_job->packet->payload[2] == 'H') {
					if (check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src) == -1)
					{
						//printf("switch %d Test %d, %d\n", switch_id, new_job->packet->src, new_job->in_port_index);
						assign_entry_in_table(MAC_Address_Table, new_job->packet->src, new_job->in_port_index, YES);
					}
					else
					{
						macAddressIndex = check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src);
						if (MAC_Address_Table[macAddressIndex].localPortTree == NO)
						{
							MAC_Address_Table[macAddressIndex].localPortTree = YES;
							
						}
					}
				}
				else if (new_job->packet->payload[2] == 'S') {
					
					if (localParent == new_job->in_port_index) {
						if (check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src) == -1)
						{
							assign_entry_in_table(MAC_Address_Table, new_job->packet->src, new_job->in_port_index, YES);
						}
						else
						{
							macAddressIndex = check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src);
							if (MAC_Address_Table[macAddressIndex].localPortTree == NO)
							{
								MAC_Address_Table[macAddressIndex].localPortTree = YES;
							}
						}
					}
					else if (new_job->packet->payload[3] == 'Y') {
						if (check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src) == -1)
						{
							assign_entry_in_table(MAC_Address_Table, new_job->packet->src, new_job->in_port_index, YES);
						}
						else
						{
							macAddressIndex = check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src);
							if (MAC_Address_Table[macAddressIndex].localPortTree == NO)
							{
								MAC_Address_Table[macAddressIndex].localPortTree = YES;
							}
						}
					}
					else {
						if (check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src) == -1)
						{
							assign_entry_in_table(MAC_Address_Table, new_job->packet->src, new_job->in_port_index, NO);
						}
						else
						{
							macAddressIndex = check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src);
							if (MAC_Address_Table[macAddressIndex].localPortTree == YES)
							{
								MAC_Address_Table[macAddressIndex].localPortTree = NO;
							}
						}
						
					}


				}
				free(new_job->packet);
				free(new_job);
				break;

			case JOB_CONTROL_SEND:
				//Code to send control packets
				in_packet = (struct packet *) malloc(sizeof(struct packet));
				new_job->packet = in_packet;

				packet_contents[0] = localRootID;
				packet_contents[1] = localRootDist;
				packet_contents[2] = 'S';
				
				for(int k = 0; k < node_port_num; k++)
				{
					val = port_to_index(MAC_Address_Table, k);
					if(localParent == MAC_Address_Table[val].dst)
					{
						packet_contents[3] = 'Y';
					} else {
						packet_contents[3] = 'N';
					}

					new_job->packet->dst 
						= k;
					new_job->packet->src = (char) switch_id;
					new_job->packet->type 
						= PKT_SWITCH_CONTROL;

					for (i=0; i<4; i++) {
						new_job->packet->payload[i] 
							= packet_contents[i];
					}
					//printf("payload[2] = %c\n", new_job->packet->payload[2]);
					new_job->packet->length = 4;
					packet_send(node_port[k], new_job->packet);
				}
				free(new_job->packet);
				free(new_job);
				break;
			default:
				//printf("Switch: %d\n", switch_id);
				destination_port = check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->dst);
				//printf("destination_port = %d\n", destination_port);
				if (destination_port == -1)
				{
					//printf("localParent = %d == %d = switch id\n", localParent, switch_id);
					if (localParent != -1 && new_job->in_port_index != check_switch_port_forwarding_table(MAC_Address_Table, localParent))
					{
						//printf("wower");
						destination_port = check_switch_port_forwarding_table(MAC_Address_Table, localParent);
						packet_send(node_port[destination_port], new_job->packet);
					}
					else
					{	
						printf("wow");
						for (i=0; i<node_port_num; i++)
						{
							val = port_to_index(MAC_Address_Table, i);
							printf("Switch: %d port %d goes to host %d: %d is %d\n", switch_id, i, MAC_Address_Table[val].dst,  MAC_Address_Table[val].localPortTree, valid_port_in_table(MAC_Address_Table, i) == YES);
							if (i != new_job->in_port_index && valid_port_in_table(MAC_Address_Table, i) == YES) {
								packet_send(node_port[i], new_job->packet);
							}
						}
					}
					
				}
				else
				{
					//printf("why\n");
					packet_send(node_port[destination_port], new_job->packet);
				}
				free(new_job->packet);
				free(new_job);
				break;
		}
	}

	// Switch sontantly sends out control packets to other switches

	new_job2 = (struct host_job *) malloc(sizeof(struct host_job));
	new_job2->type = JOB_CONTROL_SEND;
	job_q_add(&job_q, new_job2);

	/* The switch goes to sleep for 10 ms */
	usleep(TENMILLISEC);

} /* End of while loop */

}
