#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#define MAX_BLOCK_SIZE 65535
#define MAX_CONNECTIONS 5
#define TIMEOUT_S 4

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include "dhtpacket.h"
#include "sha1.h"

static const unsigned short DHT_SERVER_SHAKE = 0x413f;
static const unsigned short DHT_CLIENT_SHAKE = 0x4121;

// Utility method for creating a socket to given port on given host
// Return: the socket
int create_socket(char *host, int port);

// Utility method for creating a socket listening the given port
// Return: the socket
int create_listen_socket(int port);

// A utility method for checking if a socket has incoming data
// Return: 1 if socket has data else 0
int data_incoming(int sock);

// A utility method for safely sending data through socket
// Input: sock - sock to send to, buf - buffer to read from, len - bytes to send
// Return: -1 on failure, 0 on success
int send_all(int sock, char *buf, int *len);

// A utility method for reading any amount of data from a socket
// Input: sock - socket to read, n - how many bytes to read
// Return: pointer to received data (char[])
unsigned char *recv_all(int sock, int n);

// Receive a packet from the given socket
// Return: a DHTPacket based on the response data
DHTPacket *recv_packet(int sock);

// Perform handshake with the given socket (server or node)
// Return: 1 if handshake failed, 0 if handshake was successful
int handshake(int sock);

// Utility method for creating a handshake listener socket
// Return:  1 if client hanshake was received
//          2 if manager handshake was received
//          0 otherwise
int handshake_listener(int sock);

int get_int(int socket);

char *get_sha1(int socket);

unsigned char *get_bytes(int socket, int length);

void put_int(int socket, int value);

void put_sha1(int socket, char value[]);

void put_bytes(int socket, unsigned char value[], int length);

#endif
