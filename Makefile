#Marwan Mostafa mam024 11305332


CC=gcc
CFLAGS=-g
CPPFLAGS=-std=gnu90 -Wall -Wextra -pedantic

all: UDP_emulator server client

UDP_emulator: UDP_emulator.o liblist.a
	$(CC) -o UDP_emulator UDP_emulator.o -L. -llist

UDP_emulator.o: UDP_emulator.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o UDP_emulator.o UDP_emulator.c -I.

server: server.o
	$(CC) -o server server.o

server.o: server.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o server.o server.c 

client: client.o
	$(CC) -o client client.o

client.o: client.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o client.o client.c

liblist.a: list_adders.o list_movers.o list_removers.o
	ar -rc liblist.a list_adders.o list_movers.o list_removers.o

list_adders.o: list_adders.c list.h
	$(CC) -I./ -o list_adders.o -c $(CFLAGS) $(CPPFLAGS) list_adders.c

list_movers.o: list_movers.c list.h
	$(CC) -I./ -o list_movers.o -c $(CFLAGS) $(CPPFLAGS) list_movers.c

list_removers.o: list_removers.c list.h
	$(CC) -I./ -o list_removers.o -c $(CFLAGS) $(CPPFLAGS) \
	list_removers.c

clean:
	rm -f client
	rm -f server
	rm -f UDP_emulator
	rm -f *.o
	rm -f liblist.a

