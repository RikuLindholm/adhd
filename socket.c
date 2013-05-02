#include "socket.h"

int getInt(int socket) {
  int value;
  int size = sizeof(int);
  int n = 0;
  while (n < size)
    n += recv(socket, &value + n, size - n, 0);
  return value;
}

char *getSha1(int socket) {
  int size = 40;
  char *key = malloc(sizeof(char) * size);
  int n = 0;
  while (n < size)
    n += recv(socket, key + n, size - n, 0);
  return key;
}

unsigned char *getBytes(int socket, int length) {
  unsigned char* block = malloc(sizeof(unsigned char) * length);
  int n = 0;
  printf("Getting block of size %d\n", length);
  while (n < length) {
    printf("Trying to read bytes\n");
    n += recv(socket, block + n, length - n, 0);
    printf("Read %d bytes\n", n);
  }
  return block;
}

void putInt(int socket, int value) {
  int size = sizeof(int);
  int n = 0;
  while(n < size)
    n += send(socket, &value + n, size - n, 0);
}

void putSha1(int socket, char value[]) {
  int size = 40;
  int n = 0;
  while(n < size)
    n += send(socket, value + n, size - n, 0);
}

void putBytes(int socket, unsigned char value[], int length) {
  int n = 0;
  while(n < length)
    n += send(socket, value + n, length - n, 0);
  printf("Sent %d bytes", length);
}
