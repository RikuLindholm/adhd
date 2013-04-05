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
#include <errno.h>
#include <sys/select.h>

#define MAX_CONNECTIONS 5
#define TIMEOUT_S 4

static const unsigned short DHT_SERVER_SHAKE = 0x413f;
static const unsigned short DHT_CLIENT_SHAKE = 0x4121;

void die(char *reason) {
  fprintf(stderr, "Fatal error: %s\n", reason);
  exit(1);
}

int create_socket(char *host, int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serv_addr;
  struct hostent *server;

  server = gethostbyname(host);
  if (server == NULL)
    die("Could not find host");

  memset(&serv_addr, '\0', sizeof(serv_addr));
  memmove(&serv_addr.sin_addr.s_addr,
        server->h_addr_list[0],
        server->h_length);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    die("Could not connect to socket");

  return sock;
}

int create_listen_socket(int port) {
  int fd;
  int t;

  struct sockaddr_in a;

  a.sin_addr.s_addr = INADDR_ANY;
  a.sin_family = AF_INET;
  a.sin_port = htons(port);

  fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd == -1)
      die(strerror(errno));

  t = bind(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in));
  if (t == -1)
      die(strerror(errno));

  t = listen(fd, MAX_CONNECTIONS);
  if (t == -1)
      die(strerror(errno));        

  return fd;
}

/* Check quickly if there is more data to be read in this socket.*/
int data_incoming(int sock) {
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);
  struct timeval wait;
  wait.tv_sec = 0;  // Immediate return
  wait.tv_usec = 0;
  if (select(sock + 1, &rfds, NULL, NULL, &wait) < 0)
    die("Select error");
  if (FD_ISSET(sock, &rfds))
    return 1;
  else
    return 0;
}

char *read_all(int sock, int n) {
  fd_set rfds;
  struct timeval wait;
  int i = 0, ret;

  char *buffer = malloc((n + 1)*sizeof(char));
  if (!buffer)
    die("Memory error when receiving");
  while (i < n) {
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    // Wait untill there is more to read
    wait.tv_sec = TIMEOUT_S;
    wait.tv_usec = 0;
    if (select(sock + 1, &rfds, NULL, NULL, &wait) < 0)
      die("Select error");
    if (FD_ISSET(sock, &rfds)){
      ret = (int) read(sock, buffer + i, 44 - i);
      if (ret < 0)
      die("Read error");
      i += ret;
    } else {
      printf("Timeout\n");
      free(buffer);
      return NULL;
    }
  }
  buffer[n] = '\0';
  return buffer;
}

int DHT_handshake(int sock) {
  printf("Attempting handshake...\n");
  
  // Handshake request
  char *buffer = read_all(sock, 2);
  if (data_incoming(sock)) {
    printf("Invalid handshake (too much data)\n");
    return 1;
  }
  
  // Check handshake
  unsigned short s_shake = ((buffer[0] & 0xff) << 8) | (buffer[1] & 0xff);
  printf("Got handshake: 0x%x (%s)\n", s_shake, buffer);
  if (s_shake != DHT_SERVER_SHAKE){
    printf("Invalid handshake");
    return 1;
  }  
  
  // Handshake response
  printf("Responding to handshake...\n");
  memset(buffer, '\0', 2);
  buffer[0] = (DHT_CLIENT_SHAKE >> 8) & 0xff;
  buffer[1] = DHT_CLIENT_SHAKE & 0xff;
  if (write(sock, buffer, 2) < 0)
    die("Write error");
  free(buffer);
  return 0;
}

int main(int argc, const char * argv[]) {
  int sockfd, listener;

  if (argc < 5) {
    fprintf(stderr, "usage %s server_hostname server_port hostname port\n", argv[0]);
    exit(0);
  }

  // Create socket
  sockfd = create_socket((char *) argv[1], atoi(argv[2]));
  listener = create_listen_socket(atoi(argv[4]));

  // Execute handshake
  if (DHT_handshake(sockfd) > 0) {
    die("Handshake failed");
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
  /*
  packet = create_packet(key, key, DHT_REGISTER_BEGIN, tcp_len, (void *)tcp_addr);
  ret = transmit_packet(sockfd, packet);
  free(packet);
  if (ret > 0) {
    die("Register begin sending failed");
  } else {
    printf("Registering initializes\n");
  }

  char buffer[44];
  ret = read(sockfd, buffer, 44);
  printf("Received: %s\n", buffer);
  */

  // Register begin
  char *msg = encode_packet(key, key, DHT_REGISTER_BEGIN, tcp_len, (void *) tcp_addr);
  send(sockfd, msg, 44 + tcp_len, 0);

  // Register done
  msg = encode_packet(key, key, DHT_REGISTER_DONE, 0, NULL);
  send(sockfd, msg, 44, 0);

  // Deregister begin
  msg = encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL);
  send(sockfd, msg, 44, 0);

  // Deregister done
  msg = encode_packet(key, key, DHT_DEREGISTER_DONE, 0, NULL);
  send(sockfd, msg, 44, 0);

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
  close(sockfd);
  free(msg);
  return 0;
}
