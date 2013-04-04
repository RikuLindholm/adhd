//
//  main.c
//  P2P
//
//  Created by Woochi on 1/4/13.
//  Copyright (c) 2013 Woochi. All rights reserved.
//

#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "sha1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static const unsigned short DHT_SERVER_SHAKE = 0x413f;
static const unsigned short DHT_CLIENT_SHAKE = 0x4121;

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

  memset(&serv_addr, '\0', sizeof(serv_addr));
  memmove(&serv_addr.sin_addr.s_addr,
        server->h_addr_list[0],
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
  if (n < 0) {
    printf("Read error");
    return 1;
  }
  
  // Check handshake
  unsigned short s_shake = ((buffer[0] & 0xff) << 8) | (buffer[1] & 0xff);
  printf("Got handshake: 0x%x (%s)\n", s_shake, buffer);
  if (n != 2 || s_shake != DHT_SERVER_SHAKE){
    printf("Invalid handshake");
    return 1;
  }  
  
  // Handshake response
  printf("Responding to handshake...\n");
  memset(buffer, 0, sizeof(buffer));
  buffer[0] = (DHT_CLIENT_SHAKE >> 8) & 0xff;
  buffer[1] = DHT_CLIENT_SHAKE & 0xff;
  if (write(sock, buffer, sizeof(unsigned short)) < 0)
    return 1;
  return 0;
}

void DHT_disconnect(int s) {
  close(s);
}

int transmit_packet(int sock, DHTPacket *packet) {
  char *data = serialize_packet(packet);
  if (data == NULL)
    return 1;
  int ret = (int) write(sock, data, 44 + packet->length);
  free(data);
  if (ret < 0)
    return 1;
  return 0;
}

int main(int argc, const char * argv[]) {
  int sockfd, ret;
  DHTPacket *packet;

  if (argc < 5) {
    fprintf(stderr, "usage %s server_hostname server_port hostname port\n", argv[0]);
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
  
  // Construct the tcp_address
  // P.S remember to add -lcrypto or -lssl when compiling
  unsigned short tcp_len = strlen(argv[3]) + 2;
  char tcp_addr[tcp_len];
  unsigned short port = atoi(argv[4]);
  memcpy(tcp_addr, &port, 2);
  memcpy(tcp_addr + 2, argv[3], strlen(argv[3]));
  unsigned char *key = (unsigned char *) sha1(tcp_addr);
  
  // Perform registering
  packet = create_packet(key, key, (unsigned short) DHT_SERVER_SHAKE, tcp_len, (void *)tcp_addr);
  ret = transmit_packet(sockfd, packet);
  free(packet);
  if (ret > 0) {
    error("Register sending failed");
    exit(1);
  } else {
    printf("Registering initializes\n");
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
