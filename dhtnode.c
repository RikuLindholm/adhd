//
//  main.c
//  P2P
//
//  Created by Woochi on 1/4/13.
//  Copyright (c) 2013 Woochi. All rights reserved.
//

#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "connectionstates.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

// Include CommonCrypto or OpenSSL based on operating system
#ifdef __APPLE__
  #include <CommonCrypto/CommonDigest.h>
  #define SHA1_DIGEST_LENGTH CC_SHA1_DIGEST_LENGTH
  void sha1(unsigned char *source, unsigned int len, unsigned char *target) {
    CC_SHA1(source, len, target);
  }
#else
  #include <openssl/sha.h>
  #define SHA1_DIGEST_LENGTH SHA_DIGEST_LENGTH
  void sha1(unsigned char *source, unsigned int len, unsigned char *target) {
    SHA1(source, len, target);
  }
#endif

#define MAX_CONNECTIONS 5
#define TIMEOUT_S 4
#define STDIN 0  // file descriptor for standard input

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

unsigned char *recv_all(int sock, int n)
{
  fd_set rfds;
  struct timeval wait;
  int i = 0, ret;

  unsigned char *buffer = malloc((n + 1)*sizeof(char));
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
      ret = recv(sock, buffer + i, n - i, 0);
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

DHTPacket *recv_packet(int sock) {
  printf("Receiving a packet...\n");  
  
  // Read 44 first bytes
  unsigned char *header = recv_all(sock, 44);
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
  unsigned char *payload;
  if (length > 0) {
    payload = recv_all(sock, length);
    if (!payload){
      printf("Timeout\n");
      free(destination);
      free(origin);
      return NULL;
    }
  } else {
    payload = NULL;
  }
  
  /*
  // Check that all was read
  if (data_incoming(sock)) {
    printf("Corrupted length of received data\n");
    free(destination);
    free(origin);
    free(payload);
    return NULL;
  }
  */
  
  printf("Packet received\n");
  return create_packet(destination, origin, type, length, payload);
}

int DHT_handshake(int sock)
{
  printf("Attempting handshake...\n");
  
  // Handshake request
  unsigned char *buffer = recv_all(sock, 2);
  if (data_incoming(sock)) {
    printf("Invalid handshake (too much data)\n");
    return 1;
  }
  
  // Check handshake
  unsigned short s_shake = ((buffer[0] & 0xff) << 8) | (buffer[1] & 0xff);
  printf("Got handshake: 0x%x\n", s_shake);
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

int DHT_handshake_listener(int sock)
{
  printf("Attempting handshake...\n");
  
  // Send handshake
  unsigned char *buffer = malloc(3*sizeof(char));
  memset(buffer, '\0', 3);
  buffer[0] = (DHT_SERVER_SHAKE >> 8) & 0xff;
  buffer[1] = DHT_SERVER_SHAKE & 0xff;
  if (write(sock, buffer, 2) < 0)
    die("Write error");
  free(buffer);
  
  // Get respond
  buffer = recv_all(sock, 2);
  unsigned short s_shake = ((buffer[0] & 0xff) << 8) | (buffer[1] & 0xff);
  printf("Got handshake: 0x%x\n", s_shake);
  if (s_shake != DHT_CLIENT_SHAKE){
    printf("Invalid handshake");
    return 1;
  }
  free(buffer);
  return 0;
}

unsigned char *parse_host(unsigned char *data)
{
  unsigned char *address;
  address = malloc(strlen((char *) data) - 2);
  memcpy(address, data + 2, strlen((char *) data) - 2);
  return address;
}

int *parse_port(unsigned char *data)
{
  int *port = malloc(sizeof(int));
  memcpy(port, data, 2);
  return port;
}

int main(int argc, const char * argv[])
{
  int retval;
  int running = 1;
  int state = 0;
  fd_set master, socks;
  struct timeval tv;
  int header_len = 44;
  // struct sockaddr_storage remoteaddr; // client address
  // socklen_t addrlen;

  // Construct the tcp_address
  unsigned short tcp_len = strlen(argv[3]) + 2;
  unsigned char tcp_addr[tcp_len];
  unsigned short port = atoi(argv[4]);
  memcpy(tcp_addr, &port, 2);
  memcpy(tcp_addr + 2, argv[3], strlen(argv[3]));

  // Construct SHA1 key
  unsigned char key[SHA1_DIGEST_LENGTH];
  sha1(tcp_addr, (unsigned int) tcp_len, key);

  // Create sockets
  printf("Creating sockets\n");
  int server_sock = create_socket((char *) argv[1], atoi(argv[2]));
  int node_listener = create_listen_socket(atoi(argv[4]));
  int node_sock; // Holder for incoming node sockets

  //printf("Setting timeout values");
  // Set read timeout to zero
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&master); //Reset master socket set
  FD_ZERO(&socks); //Reset temp socket set
	FD_SET(STDIN, &master); // Add standard input to master set
	FD_SET(server_sock, &master); // Add server sock master set
	FD_SET(node_listener, &master); // Add server listener to master set

  // Perform initial handshake
  printf("Performing handshake...\n");
  DHT_handshake(server_sock);

  printf("Starting the main loop...\n");
  while (running) {
    socks = master; // Reset socks from master

    // Count sockets with incoming data
    if (state == REGISTERED)
      retval = select(node_listener + 1, &socks, NULL, NULL, NULL);
    else
      retval = select(node_listener + 1, &socks, NULL, NULL, &tv);

    // Socket data handler
    if (retval) {
      printf("Some sockets are hot: ");
      // Check standard input
      if (FD_ISSET(STDIN, &socks)) {
        printf("Exiting\n");
        running = 0;
      }

      if (FD_ISSET(server_sock, &socks)) {
        printf("got connection from server\n");
        DHTPacket *pkt = recv_packet(server_sock);
        if (pkt->type == DHT_REGISTER_FAKE_ACK)
          state = CONNECTED;
        else if (pkt->type == DHT_REGISTER_BEGIN) {
          printf("Another node has connected {%s, %d} - sending ack\n",
                  parse_host(pkt->data), *parse_port(pkt->data));
          node_sock = create_socket((char *) parse_host(pkt->data), *parse_port(pkt->data));
          // Perform handshake
          DHT_handshake(node_sock);
          int data_len = header_len + tcp_len;
          send_all(node_sock,
                    encode_packet(key, key, DHT_REGISTER_ACK, tcp_len, (void *) tcp_addr),
                    &data_len);
        } else if (pkt->type == DHT_DEREGISTER_ACK) {
          // Send DEREGISTER_BEGIN to neighbour nodes
          unsigned char address1[tcp_len];
          unsigned char address2[tcp_len];
          memcpy(address1, pkt->data, tcp_len);
          memcpy(address1, pkt->data + tcp_len, tcp_len);
          int neighbour_socks[2] = {
            create_socket((char *) parse_host(address1), *parse_port(address1)),
            create_socket((char *) parse_host(address2), *parse_port(address2))
          };
          for (int i = 0; i < 2; i++) {
            send_all(neighbour_socks[i],
                      encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL),
                        &header_len);
            close(neighbour_socks[i]);
          }
        } else if (pkt->type == DHT_DEREGISTER_DONE) {
          running = 0;
        }
        free(pkt);
      }

      if (FD_ISSET(node_listener, &socks)) {
        printf("got connection from another node\n");
        node_sock = accept(node_listener, NULL, NULL);
        if (node_sock < 0)
          die("Could not create new node socket");
        else {
          printf("New node connection\n");
          // Perform handshake
          DHT_handshake_listener(node_sock);
          DHTPacket *pkt = recv_packet(node_sock);
          if (pkt->type == DHT_REGISTER_ACK)
            state++;
          else if (pkt->type == DHT_DEREGISTER_BEGIN) {
            // Send acknowledgement of neighbour deregister to server
            send_all(server_sock,
                      encode_packet(pkt->origin, key, DHT_DEREGISTER_DONE, 0, NULL),
                        &header_len);
          }
          free(pkt);
        }
        close(node_sock);
      }

    } /*else {
      printf("No sockets are hot - Press any key to disconnect\n");
    }*/

    if (!state) {
      // Send REGISTER_BEGIN
      printf("Sending REGISTER_BEGIN\n");
      int data_len = header_len + tcp_len;
      send_all(server_sock,
                encode_packet(key, key, DHT_REGISTER_BEGIN, tcp_len, (void *) tcp_addr),
                  &data_len);
      state++;
    }

    if (state == CONNECTED) {
      // Send REGISTER_DONE
      printf("Sending REGISTER_DONE\n");
      send_all(server_sock,
                encode_packet(key, key, DHT_REGISTER_DONE, 0, NULL),
                  &header_len);
      state = REGISTERED;
    }

    if (state == DEREGISTERING) {
      // Send DEREGISTER_BEGIN
      printf("Sending DEREGISTER_BEGIN\n");
      send_all(server_sock,
                encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL),
                  &header_len);
      state++;
    }
  }

  // Close socket
  close(server_sock);
  close(node_listener);
  return 0;
}
