# Make file

net367: 
	make net367-server
	make net367-client

net367-server: host.o packet.o man.o main.o net_host.o switch.o
	gcc -o net367-server host.o man.o main.o net_host.o packet.o switch.o

net367-client: host.o packet.o man.o main.o net_client.o switch.o
	gcc -o net367-client host.o man.o main.o net_client.o packet.o switch.o

main.o: main.c
	gcc -c main.c

host.o: host.c 
	gcc -c host.c

man.o:  man.c
	gcc -c man.c

net_client.o:  net_client.c
	gcc -c net_client.c

net_host.o:  net_host.c
	gcc -c net_host.c

packet.o:  packet.c
	gcc -c packet.c

switch.o: switch.c
	gcc -c switch.c

clean:
	rm *.o

