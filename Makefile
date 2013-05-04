CFLAGS=-c -g -Wall -Werror -std=c99 -pedantic
LDFLAGS=-lcrypto
CC = gcc
LD = gcc
JCC = javac
OBJS = dhtnode.o dhtpacket.o socket.o list_node.o sha1.o
PROG = dhtnode
JOBJS = gui/IconButton.class gui/Connection.class gui/FileMessage.class gui/GUI.class
JPROG = gui/GUI.java

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

gui: $(JOBJS)
	java gui.GUI

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

dhtnode.o: dhtnode.c dhtpacket.c socket.c list_node.c sha1.c dhtpackettypes.h connectionstates.h
	$(CC) $(CFLAGS) dhtnode.c $(LDFLAGS)

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

sha1.o: sha1.c
	$(CC) $(CFLAGS) sha1.c

socket.o: socket.c dhtpacket.c
	$(CC) $(CFLAGS) socket.c

list_node.o: list_node.c dhtpacket.c
	$(CC) $(CFLAGS) list_node.c

gui/GUI.class: gui/GUI.java
	$(JCC) $(JPROG)

gui/IconButton.class: gui/IconButton.java
	$(JCC) gui/IconButton.java

gui/ConnectDialog.class: gui/ConnectDialog.java
	$(JCC) gui/ConnectDialog.java

gui/FileMessage.class: gui/FileMessage.java
	$(JCC) gui/FileMessage.java

gui/Connection.class: gui/Connection.java
	$(JCC) gui/Connection.java

clean:
	/bin/rm -f *.o dhtnode gui/*.class

run:
	./$(PROG) 127.0.0.1 3200 127.0.0.1 50000

run-gui:
	java gui.GUI
