//
//  main.c
//  P2P
//
//  Created by Woochi on 1/4/13.
//  Copyright (c) 2013 Woochi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/sha.h>

// TODO: Are the codes predefined somewhere?
static const unsigned short DHT_SERVER_SHAKE =      0x413f;
static const unsigned short DHT_CLIENT_SHAKE =      0x4121;
static const unsigned short DHT_REGISTER_BEGIN =    0x4122;
static const unsigned short DHT_REGISTER_ACK =      0x4123;
static const unsigned short DHT_REGISTER_FAKE_ACK = 0x4124;
static const unsigned short DHT_REGISTER_DONE =     0x4125;
static const unsigned short DHT_DEREGISTER_BEGIN =  0x4126;
static const unsigned short DHT_DEREGISTER_DONE =   0x4127;
static const unsigned short DHT_DEREGISTER_ACK =    0x4128;
static const unsigned short DHT_DEREGISTER_DENY =   0x4129;

typedef struct DHTPacket {
  unsigned char destination;
  unsigned char origin;
  unsigned int type;
  unsigned int length;
  void *data;
} DHTPacket;

DHTPacket *create_packet(unsigned char destination, unsigned char origin,
                                unsigned int type, unsigned int length,
                                void* data) {
  DHTPacket *packet = malloc(sizeof(struct DHTPacket));
  packet->destination = destination;
  packet->origin = origin;
  packet->type = type;
  packet->length = length;
  if (data != NULL)
    packet->data = malloc(sizeof(data));

  return packet;
};

char *serialize_packet(unsigned char destination, unsigned char origin,
                         unsigned int type, unsigned int length,
                         void* data) {
  // TODO: Implement serialization
};

DHTPacket *deserialize_packet(char *data) {
  // TODO: Implement deserialization
};

void error(const char *msg) {
  perror(msg);
  exit(0);
}

int DHT_connect(int sockfd, const char *host, int port) {
  struct sockaddr_in serv_addr;
  struct hostent *server;

  server = gethostbyname(host);
  if (server == NULL)
    return 1;

  bzero((char *) &serv_addr, sizeof(serv_addr));
  bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    return 1;

  return 0;
}

int DHT_handshake(int sock) {
  printf("Attempting handshake...\n");
  int n;
  char buffer[16];

  // Handshake request
  memset(buffer, 0, sizeof(buffer));
  n = (int) read(sock, buffer, sizeof(buffer) - 1);
  printf("Got handshake: %s\n", buffer);

  if (n < 0) {
    printf("Read error");
    return 1;
  }

  // Handshake response
  printf("Responding to handshake...\n");
  memset(buffer, 0, sizeof(buffer));
  if (write(sock, (char *) DHT_CLIENT_SHAKE, strlen(buffer)) < 0)
    return 1;

  return 0;
}

void DHT_disconnect(int s) {
  close(s);
}

int main(int argc, const char * argv[]) {
  int sockfd;

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("Could not create socket");
    exit(1);
  }

  // Connect socket to server
  if (DHT_connect(sockfd, argv[1], atoi(argv[2])) > 0) {
    error("Could not connect to server");
    exit(1);
  } else {
    printf("Connected!\n");
  }

  // Execute handshake
  if (DHT_handshake(sockfd) > 0) {
    error("Handshake failed");
    exit(1);
  } else {
    printf("Handshake was successful\n");
  }

  // TODO: Registering client
  // Send DHT_REGISTER_BEGIN to server
  // Wait DHT_REGISTER_ACK from neighbours (x2)
  // OR wait for DHT_REGISTER_FAKE_ACK from server
  // Send DHT_REGISTER_DONE to server

  // TODO: Deregistering client
  // Send DHT_DEREGISTER_BEGIN to server
  // Wait for DHT_DEREGISTER_ACK from server
  // Send DHT_DEREGISTER_BEGIN to neighbours (x2)
  // Wait for DHT_DEREGISTER_DONE from server
  // Close connection

  // Close socket
  DHT_disconnect(sockfd);
  return 0;
}
