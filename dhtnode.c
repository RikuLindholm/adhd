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

/* Read repeatedly from given socket until the requested n bytes are read or
 * the timeout is reached when waiting for more input.
 */
char *read_until_n(int sock, int n) {
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

int DHT_handshake(int sock) {
  printf("Attempting handshake...\n");
  
  // Handshake request
  char *buffer = read_until_n(sock, 2);
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

int transmit_packet(int sock, DHTPacket *packet) {
  char *data = serialize_packet(packet);
  if (data == NULL)
    die("Out of memory");
  printf("Sending packet: %s", data);
  int tot = 0, ret;
  while (tot < 44 + packet->length) {
    ret = (int) send(sock, data + tot, 44 - tot + packet->length, 0);
    if (ret < 0)
      die(strerror(errno));
    tot += ret;
  }
  free(data);
  return 0;
}

DHTPacket *recv_packet(int sock) {
  printf("Receiving a packet...\n");  
  
  // Read 44 first bytes
  char *header = read_until_n(sock, 44);
  if (!header){
    printf("Timeout\n");
    return NULL;
  }
  unsigned char *destination = malloc(20*sizeof(char));
  if (!destination)
    die("Memory error");
  memcpy(destination, header, 20);
  unsigned char *origin = malloc(20*sizeof(char));
  if (!origin)
    die("Memory error");
  memcpy(origin, header + 20, 20);
  unsigned short type = ((header[40] & 0xff) << 8) | (header[41] & 0xff);
  unsigned short length = ((header[42] & 0xff) << 8) | (header[43] & 0xff);
  free(header);
  
  // Read the payload
  char *payload;
  if (length > 0) {
    payload = read_until_n(sock, length);
    if (!payload){
      printf("Timeout\n");
      free(destination);
      free(origin);
      return NULL;
    }
  } else {
    payload = NULL;
  }
  
  // Check that all was read
  if (data_incoming(sock)) {
    printf("Corrupted length of received data\n");
    free(destination);
    free(origin);
    free(payload);
    return NULL;
  }
  
  printf("Packet received\n");
  return create_packet(destination, origin, type, length, (void *)payload);
}


int create_listen_socket(unsigned short port) {
  int sock_listen, ret;
  struct sockaddr_in a;

  a.sin_addr.s_addr = INADDR_ANY;
  a.sin_family = AF_INET;
  a.sin_port = htons(port);

  sock_listen = socket(PF_INET, SOCK_STREAM, 0);    // Huom! PF_INET vai AF_INET
  if (sock_listen == -1)
      die("Could not create a listening socket to the given port");

  ret = bind(sock_listen, (struct sockaddr *)(&a), sizeof(struct sockaddr_in));
  if (ret == -1)
      die("Could not bind the listening socket");

  ret = listen(sock_listen, MAX_CONNECTIONS);
  if (ret == -1)
      die("Could not set the socket to listen");        

  return sock_listen;
}

int main(int argc, const char * argv[]) {
  int sock_server, sock_listen ret;

  if (argc < 5) {
    fprintf(stderr, "usage %s server_hostname server_port hostname port\n", argv[0]);
    exit(0);
  }

  // Create sockets
  sock_server = create_socket((char *) argv[1], atoi(argv[2]));
  sock_listen = create_listen_socket(atoi(argv[4]));

  // Execute handshake
  if (DHT_handshake(sock_server) > 0) {
    die("Handshake failed");
  } else {
    printf("Handshake was successful\n");
  }
  
  // Construct the tcp_address
  // P.S remember to add -lcrypto or -lssl when compiling
  unsigned short tcp_len = strlen(argv[3]) + 2;
  char tcp_addr[tcp_len];
  unsigned short port = atoi(argv[4]);
  tcp_addr[0] = (port >> 8) & 0xff;     // In network byte order
  tcp_addr[1] = port & 0xff;
  memcpy(tcp_addr + 2, argv[3], strlen(argv[3]));
  unsigned char *key = (unsigned char *) sha1(tcp_addr);
  
  // Perform registering
  DHTPacket *packet = create_packet(key, key,
                        DHT_REGISTER_BEGIN, tcp_len, (void *)tcp_addr);
  ret = transmit_packet(sock_server, packet);
  free(key);
  free(packet);
  if (ret > 0) {
    die("Register sending failed");
  } else {
    printf("Registering initializing\n");
  }
  
  // Accept connections
  fd_set rfds;
  int nfds, running = 1;
  if (sock_listen > sock_server)
    nfds = sock_listen + 1;
  else
    nfds = sock_server + 1;
  while (running) {
    FD_ZERO(&rfds);
    FD_SET(0, &rfds); /* Standard input */
    FD_SET(sock_listen, &rfds);
    FD_SET(sock_server, &rfds);
    // Wait for any input in sockets or standard input
    ret = select(nfds, &rfds, NULL, NULL, NULL);
    if (ret == -1)
      die("Select failed");
    else if (ret) {
      if (FD_ISSET(0, &rfds))
        running = 0; /* Exit when console input available */
      if (FD_ISSET(sock_server, &rfds)) {
        // Server responds
        printf("Server responds\n");
        DHTPacket *packet = recv_packet(sock_server);
        if (packet) {
          if (packet->type == DHT_REGISTER_FAKE_ACK) {
            // This is the only node
            // TODO: Forward the information out of the while loop
            running = 0;
          } else {
            printf("Unexpected packet from server\n");
          }
          free(packet->destination);
          free(packet->origin);
          free(packet->data);
          free(packet);
        }
      }
      if (FD_ISSET(sock_listen, &rfds)) {
        
        // TODO: Neighbour connects
        
        // Dummy
        struct sockaddr_in tempaddr;
        unsigned int addrlen = 0;
        int tempfd = accept(sock_listen, (struct sockaddr *)&tempaddr,
                &addrlen);
        write(tempfd, "Hello!\n", 7); /* Answer politely then close. */
        close(tempfd);
        
      }
    }
  }
  
  /*
  // Register begin
  char *msg = encode_packet(key, key, DHT_REGISTER_BEGIN, tcp_len, (void *) tcp_addr);
  send(sockfd, msg, 44, 0);
  */
  
  // Register done
  msg = encode_packet(key, key, DHT_REGISTER_DONE, tcp_len, (void *)tcp_addr);
  send(sockfd, msg, 44, 0);

  // Deregister begin
  msg = encode_packet(key, key, DHT_DEREGISTER_BEGIN, tcp_len, (void *)tcp_addr);
  send(sockfd, msg, 44, 0);

  // Deregister done
  msg = encode_packet(key, key, DHT_DEREGISTER_DONE, tcp_len, (void *)tcp_addr);
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
  close(sock_server);
  free(msg);
  return 0;
}
