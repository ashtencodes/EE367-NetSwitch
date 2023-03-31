#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "main.h"
#include "packet.h"
#include "net.h"
#include "host.h"

void packet_send(struct net_port *port, struct packet *p)
{

char msg[PAYLOAD_MAX+4];
int i;
int send_num;
//printf("Port type: %d\n", port->type);

if (port->type == PIPE) {
	//printf("sending to pipe\n");
	msg[0] = (char) p->src; 
	msg[1] = (char) p->dst;
	msg[2] = (char) p->type;
	msg[3] = (char) p->length;
	for (i=0; i<p->length; i++) {
		msg[i+4] = p->payload[i];
	}
	if(send_num = write(port->pipe_send_fd, msg, p->length+4) < 0){
		printf("Send failed\n");
	} else {
		//printf("Sent %d bits\n",send_num);
	}
}
	

if (port->type == SOCKET) {
	msg[0] = (char) p->src; 
	msg[1] = (char) p->dst;
	msg[2] = (char) p->type;
	msg[3] = (char) p->length;
	for (i=0; i<p->length; i++) {
		msg[i+4] = p->payload[i];
	}
	//printf("Sending packet with p->length: %d\n",p->length);
	if((send_num = send(port->pipe_send_fd, msg, p->length+4, 0)) < 0){
		printf("Send failed\n");
	} else {
		//printf("Sent %d bits\n",send_num);
	}
}

return;
}

int packet_recv(struct net_port *port, struct packet *p)
{
char msg[PAYLOAD_MAX+4];
int n;
int i;
	
if (port->type == PIPE) {
	n = read(port->pipe_recv_fd, msg, PAYLOAD_MAX+4);
	//printf("reading from pipe\n");
	if (n>0) {
		p->src = (char) msg[0];
		p->dst = (char) msg[1];
		p->type = (char) msg[2];
		p->length = (int) msg[3];
		for (i=0; i<p->length; i++) {
			p->payload[i] = msg[i+4];
		}
	}
}

if (port->type == SOCKET) {
	n = recv(port->pipe_recv_fd, msg, PAYLOAD_MAX+4, 0);
	//printf("reading from socket\n");
	if (n>0) {
		//printf("Read a packet from the socket!\n");
		p->src = (char) msg[0];
		p->dst = (char) msg[1];
		//printf("p->dst: %d\n",p->dst);
		p->type = (char) msg[2];
		p->length = (int) msg[3];
		for (i=0; i<p->length; i++) {
			p->payload[i] = msg[i+4];
		}
	}
}

return(n);
}


