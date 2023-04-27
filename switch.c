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
#include "packet.h"
#include "switch.h"

#define TENMILLISEC 10000   /* 10 millisecond sleep */
#define MAX_PORT_TABLE_LENGTH 100

struct packet *new_packet;

enum localPortTreeValue
{
	NO,
	YES
};

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

int assign_entry_in_table(struct switch_port_forwarding table [], char dst, int src_port)
{
    for (int i = 0; i < MAX_PORT_TABLE_LENGTH; i++)
    {
        if (table[i].valid == NotValid)
        {
            table[i].valid = Valid;
            table[i].dst = dst;
            table[i].port = src_port;
            return i;
        }
    }
	return -1;
}
/* Initialize job queue */
void job_q_init(struct switch_job_queue *j_q)
{
j_q->occ = 0;
j_q->head = NULL;
j_q->tail = NULL;
}
struct host_job *job_q_remove(struct switch_job_queue *j_q)
{
struct host_job *j;

if (j_q->occ == 0) return(NULL);
j = j_q->head;
j_q->head = (j_q->head)->next;
j_q->occ--;
return(j);
}

void job_q_add(struct switch_job_queue *j_q, struct switch_job *j)
{
if (j_q->head == NULL ) {
	j_q->head = j;
	j_q->tail = j;
	j_q->occ = 1;
}
else {
	(j_q->tail)->next = j;
	j->next = NULL;
	j_q->tail = j;
	j_q->occ++;
}
}

int job_q_num(struct switch_job_queue *j_q)
{
return j_q->occ;
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
struct switch_job *new_job;
struct switch_job *new_job2;

struct switch_job_queue job_q;

struct switch_port_forwarding MAC_Address_Table[MAX_PORT_TABLE_LENGTH];
enum localPortTreeValue localPortTree[MAX_PORT_TABLE_LENGTH];


/*
 * Initialize Table 
 * Sets the entries as not valid until assigned
 */
for (int i=0; i<MAX_PORT_TABLE_LENGTH; i++)
	MAC_Address_Table[i].valid = NotValid;

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

char packet_contents[3];

while(1) {

	for (k = 0; k < node_port_num; k++) { /* Scan all ports */

		in_packet = (struct packet *) malloc(sizeof(struct packet));
		n = packet_recv(node_port[k], in_packet);

		if (n > 0) {

			new_job = (struct switch_job *)
				malloc(sizeof(struct switch_job));
			new_job->in_port_index = k;
			new_job->packet = in_packet;

			switch(in_packet->type) {

				case (char) PKT_SWITCH_CONTROL: 
					new_job->type = JOB_CONTROL_RECV;
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

		switch(new_job->type) {

		/* Send packets on all ports */	
		case JOB_CONTROL_RECV:
			//TO DO: Add code to handle control packets

			if(new_job->packet->payload[2] == 'S')
			{
				if(new_job->packet->payload[0] < localRootID)
				{
					localRootID = new_job->packet->payload[0];
					localParent = k;
					localRootDist = new_job->packet->payload[1] + 1;
				}
				else if(new_job->packet->payload[0] == localRootID)
				{
					if(new_job->packet->payload[1] + 1 < localRootDist)
					{
						localRootDist = new_job->packet->payload[1] + 1;
						localParent = k;
					}
				}
			}
			if (new_job->packet->payload[2] == 'H') localPortTree[k] = YES;
			else if (new_job->packet->payload[2] == 'S')
			{
				if (localParent == k) localPortTree[k] = YES;
				else if (new_job->packet->payload[3] == 'Y') localPortTree[k] = YES;
				else localPortTree[k] = NO;
			}

			free(new_job->packet);
			free(new_job);
			break;

		case JOB_CONTROL_SEND:
			//Code to send control packets

			packet_contents[0] = localRootID;
			packet_contents[1] = localRootDist;
			packet_contents[2] = 'S';

			for(int i = 0; i < MAX_PORT_TABLE_LENGTH; i++)
			{
				if (MAC_Address_Table[i].valid == Valid)
				{
					if(MAC_Address_Table[n].isParent == 1)
					{
						packet_contents[3] = 'Y';
					} else {
						packet_contents[3] = 'N';
					}

					new_packet = (struct packet *) 
							malloc(sizeof(struct packet));
					new_packet->dst 
						= NULL;
					new_packet->src = (char) switch_id;
					new_packet->type 
						= PKT_SWITCH_CONTROL;

					for (i=0; i<4; i++) {
						new_packet->payload[i] 
							= packet_contents[i];
					}

					new_packet->length = 4;

					packet_send(node_port[MAC_Address_Table[i].port], new_packet);
				}
			}

			free(new_job->packet);
			free(new_job);
			break;
		}

		if (check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->src) == -1)
		{
			assign_entry_in_table(MAC_Address_Table, new_job->packet->src, new_job->in_port_index);
		}

        int destination_port = check_switch_port_forwarding_table(MAC_Address_Table, new_job->packet->dst);

		if (destination_port == -1)
		{
            for (i=0; i<node_port_num; i++)
			{
				//printf("%d\n",node_port[i]->pipe_send_fd);
				if (i != new_job->in_port_index) {
					packet_send(node_port[i], new_job->packet);
				}
			}
		}
		else
		{
			packet_send(node_port[destination_port], new_job->packet);
		}

		free(new_job->packet);
		free(new_job);
	}

	// Switch sontantly sends out control packets to other switches

	new_job2 = (struct switch_job *) malloc(sizeof(struct switch_job));
	new_job2->type = JOB_CONTROL_SEND;
	job_q_add(&job_q, new_job2);

	/* The switch goes to sleep for 10 ms */
	usleep(TENMILLISEC);

} /* End of while loop */

}
