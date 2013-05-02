#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#define MAX_BLOCK_SIZE 65535

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include "sha1.h"

int getInt(int socket);

char *getSha1(int socket);

unsigned char *getBytes(int socket, int length);

void putInt(int socket, int value);

void putSha1(int socket, char value[]);

void putBytes(int socket, unsigned char value[], int length);

#endif
