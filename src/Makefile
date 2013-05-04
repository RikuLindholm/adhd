CFLAGS=-c -g -Wall -Werror -std=c99 -pedantic
LDFLAGS=-lcrypto
CC = gcc
TARGET = dhtnode
OUTPUT = ../bin/$(TARGET)
OBJS = \
	dhtnode.c \
	dhtpacket.c \
	socket.c \
	list_node.c \
	sha1.c

all: objs
	$(CC) $(OBJS) -o $(OUTPUT) $(LDLAGS)

objs: $(OBJS:.c=.o)

.c.o:
	$(CC) $< -o $@ $(CFLAGS)

clean:
	/bin/rm *.o
