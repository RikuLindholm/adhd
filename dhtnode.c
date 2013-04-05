//
//  main.c
//  P2P
//
//  Created by Woochi on 1/4/13.
//  Copyright (c) 2013 Woochi. All rights reserved.
//

#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

// Include CommonCrypto or OpenSSL based on operating system
#ifdef __APPLE__ && __MACH__
  #include <CommonCrypto/CommonDigest.h>
  void sha1(unsigned char *source, unsigned int len, unsigned char *target) {
    CC_SHA1(source, len, target);
  }
#else
  #include <openssl/sha.h>
  void sha1(source char *source, unsigned int len, unsigned char *target) {
    SHA1(origin, len, target);
  }
#endif

#define MAX_CONNECTIONS 5
#define TIMEOUT_S 4

static const unsigned short DHT_SERVER_SHAKE = 0x413f;
static const unsigned short DHT_CLIENT_SHAKE = 0x4121;

void die(char *reason)
{
  fprintf(stderr, "Fatal error: %s\n", reason);
  exit(1);
}

int create_socket(char *host, int port)
{
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

int create_listen_socket(int port)
{
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
int data_incoming(int sock)
{
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

int send_all(int sock, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(sock, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}

char *read_all(int sock, int n)
{
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
      ret = recv(sock, buffer + i, 44 - i, 0);
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

int DHT_handshake(int sock)
{
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

int main(int argc, const char * argv[])
{
  int sockfd, listener;

  if (argc < 5)
    die("usage %s server_hostname server_port hostname port");

  // Create socket
  sockfd = create_socket((char *) argv[1], atoi(argv[2]));
  listener = create_listen_socket(atoi(argv[4]));

  // Execute handshake
  if (DHT_handshake(sockfd) > 0)
    die("Handshake failed");
  else
    printf("Handshake was successful\n");

  // Construct the tcp_address
  // P.S remember to add -lcrypto or -lssl when compiling
  int tcp_len = strlen(argv[3]) + 2;
  char tcp_addr[tcp_len];
  unsigned short port = atoi(argv[4]);
  memcpy(tcp_addr, &port, 2);
  memcpy(tcp_addr + 2, argv[3], strlen(argv[3]));

  // Construct SHA1 key
  unsigned char key[CC_SHA1_DIGEST_LENGTH];
  CC_SHA1(tcp_addr, (unsigned int) tcp_len, key);
  printf("Sending with key: %s", key);

  // Register begin
  int header_len = 44;
  int data_len = header_len + tcp_len;
  char *msg = encode_packet(key, key, DHT_REGISTER_BEGIN, tcp_len, (void *) tcp_addr);
  send_all(sockfd, msg, &data_len);

  // Accept connections
  fd_set rfds;
  int ret, nfds, running = 1;
  if (listener > sockfd)
    nfds = listener + 1;
  else
    nfds = listener + 1;

  while (running) {
    FD_ZERO(&rfds);
    FD_SET(0, &rfds); /* Standard input */
    FD_SET(listener, &rfds);
    FD_SET(listener, &rfds);
    // Wait for any input in sockets or standard input
    ret = select(nfds, &rfds, NULL, NULL, NULL);
    if (ret == -1)
      die("Select failed");
    else if (ret) {
      if (FD_ISSET(0, &rfds))
        running = 0; /* Exit when console input available */
      if (FD_ISSET(sockfd, &rfds)) {
        // Server responds
        printf("Server responds\n");
        DHTPacket *packet = recv_packet(sockfd);
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
      if (FD_ISSET(listener, &rfds)) {
        
        // TODO: Neighbour connects
        
        // Dummy
        struct sockaddr_in tempaddr;
        unsigned int addrlen = 0;
        int tempfd = accept(listener, (struct sockaddr *)&tempaddr,
                &addrlen);
        write(tempfd, "Hello!\n", 7); /* Answer politely then close. */
        close(tempfd);
        
      }
    }
  }

  // Register done
  msg = encode_packet(key, key, DHT_REGISTER_DONE, 0, NULL);
  send_all(sockfd, msg, &header_len);

  // Deregister begin
  msg = encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL);
  send_all(sockfd, msg, &header_len);

  // Deregister done
  msg = encode_packet(key, key, DHT_DEREGISTER_DONE, 0, NULL);
  send_all(sockfd, msg, &header_len);

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
  close(listener);
  free(msg);
  return 0;
}
