CFLAGS=-c -g -Wall -Werror -std=c99 -pedantic
LDFLAGS=-lcrypto
CC = gcc
LD = gcc
JCC = javac
OBJS = dhtnode.o dhtpacket.o
PROG = dhtnode
JOBJS = gui/IconButton.class gui/GUI.class
JPROG = gui/GUI.java

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

gui: $(JOBJS)
	$(JCC) $(JPROG)
	java gui.GUI

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

dhtnode.o: dhtnode.c dhtpacket.c dhtpackettypes.h connectionstates.h
	$(CC) $(CFLAGS) dhtnode.c $(LDFLAGS)

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

gui/IconButton.class: gui/IconButton.java
	$(JCC) gui/IconButton.java

gui/GUI.class: gui/GUI.java
	$(JCC) gui/GUI.java

clean:
	/bin/rm -f *.o dhtnode gui/*.class

run:
	./$(PROG) 127.0.0.1 3200 127.0.0.1 50000

run-gui:
	java gui.GUI
