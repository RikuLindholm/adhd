CFLAGS=-c -g -Wall -Werror -std=c99 -pedantic
LDFLAGS=-lgcrypt
CC = gcc
LD = gcc
OBJS = dhtnode.o dhtpacket.o
PROG = dhtnode

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

dhtnode.o: dhtnode.c dhtpacket.c dhtpackettypes.h
	$(CC) $(CFLAGS) dhtnode.c

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

clean:
	/bin/rm -f *.o dhtnode

run:
	./$(PROG) 127.0.0.1 3200 127.0.0.1 50000
