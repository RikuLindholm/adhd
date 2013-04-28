CFLAGS=-c -g -Wall -Werror -std=c99 -pedantic
LDFLAGS=-lcrypto
CC = gcc
LD = gcc
JCC = javac
OBJS = dhtnode.o dhtpacket.o
PROG = dhtnode
JOBJS = gui/IconButton.java gui/GUI.java

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

gui: $(JOBJS)
	$(JCC) $(JOBJS)
	java gui.GUI

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

dhtnode.o: dhtnode.c dhtpacket.c dhtpackettypes.h connectionstates.h
	$(CC) $(CFLAGS) dhtnode.c $(LDFLAGS)

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

clean:
	/bin/rm -f *.o dhtnode

run:
	./$(PROG) 127.0.0.1 3200 127.0.0.1 50000
