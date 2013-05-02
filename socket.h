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

unsigned char *getBlock(int socket, int length);

#endif
