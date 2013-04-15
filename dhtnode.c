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

// Utility method for printing error messages and exiting
void die(char *reason)
{
  fprintf(stderr, "Fatal error: %s\n", reason);
  exit(1);
}

// Utility method for creating a socket to given port on given host
// Return: the socket
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

// Utility method for creating a socket listening the given port
// Return: the socket
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

// A utility method for checking if a socket has incoming data
// Return: 1 if socket has data else 0
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

// A utility method for safely sending data through socket
// Input: sock - sock to send to, buf - buffer to read from, len - bytes to send
// Return: -1 on failure, 0 on success
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

    return (n == -1 ? -1 : 0);
}

// A utility method for reading any amount of data from a socket
// Input: sock - socket to read, n - how many bytes to read
// Return: pointer to received data (char[])
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

// Receive a packet from the given socket
// Return: a DHTPacket based on the response data
DHTPacket *recv_packet(int sock)
{  
  // Read 44 first bytes == header
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

  DHTPacket *pkt = create_packet(destination, origin, type, length, payload);
  if (payload != NULL)
    free(payload);
  return pkt;
}

// Perform handshake with the given socket (server or node)
// Return: 1 if handshake failed, 0 if handshake was successful
int handshake(int sock)
{ 
  // Handshake request
  unsigned char *buffer = recv_all(sock, 2);
  if (data_incoming(sock)) {
    printf("Invalid handshake (too much data)\n");
    return 1;
  }

  // Check handshake
  unsigned short s_shake = ((buffer[0] & 0xff) << 8) | (buffer[1] & 0xff);
  if (s_shake != DHT_SERVER_SHAKE){
    printf("Invalid handshake\n");
    return 1;
  }
  
  // Handshake response
  memset(buffer, '\0', 2);
  buffer[0] = (DHT_CLIENT_SHAKE >> 8) & 0xff;
  buffer[1] = DHT_CLIENT_SHAKE & 0xff;
  if (write(sock, buffer, 2) < 0)
    die("Write error");
  free(buffer);
  return 0;
}

// Utility method for creating a handshake listener socket
// Return: 1 if handshake failed, else 0
int handshake_listener(int sock)
{
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
  if (s_shake != DHT_CLIENT_SHAKE){
    printf("Invalid handshake\n");
    return 1;
  }
  free(buffer);
  return 0;
}

// Parse host from a given TCP address (port+ host)
// Return: pointer to host char array
unsigned char *parse_host(unsigned char *data)
{
  int len = strlen((char *) data) - 2;
  unsigned char *address = malloc(len + 1);
  memcpy(address, data + 2, len);
  address[len] = '\0';
  return address;
}

// Parse port from from a given TCP address (port+host)
int parse_port(unsigned char *data)
{
  return (int)(((data[0] & 0xff) << 8) | (data[1] & 0xff));
}

// Places the difference between keys a and b: |a - b| into array result.
// All arrays must be of length SHA1_DIGEST_LENGTH.
void difference(unsigned char *result, unsigned char *a, unsigned char *b)
{
  // Check if a is bigger than b. If not -> switch
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (b[i] > a[i]) {
      unsigned char *temp = a;
      a = b;
      b = temp;
      break;
    } else if (b[i] < a[i])
      break;
  }
  
  short carry = 0;
  short v1;
  // Perform subtraction a - b
  for (int i = SHA1_DIGEST_LENGTH - 1; i >= 0; i--) {
    v1 = (short)a[i] - carry - (short)b[i];
    carry = 0;
    if (v1 < 0){
      carry = 1;
      v1 += 0x10000;
    }
    result[i] = v1 & 0xff;
  }  
}

int main(int argc, const char * argv[])
{
  int retval;
  int running = 1;
  int state = 0;
  fd_set master, socks;
  struct timeval tv;
  int header_len = 44;
  unsigned char *node_host;
  int node_port;
  
  // struct sockaddr_storage remoteaddr; // client address
  // socklen_t addrlen;

  // Construct the tcp_address
  unsigned short tcp_len = strlen(argv[3]) + 2;
  unsigned char tcp_addr[tcp_len];
  unsigned short port = atoi(argv[4]);
  tcp_addr[0] = (port >> 8) & 0xff;     // In network byte order
  tcp_addr[1] = port & 0xff;
  memcpy(tcp_addr + 2, argv[3], strlen(argv[3]));
  int std_input;

  // Construct SHA1 key
  unsigned char key[SHA1_DIGEST_LENGTH];
  sha1(tcp_addr, (unsigned int) tcp_len, key);

  // Create sockets
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

  // Perform initial handshake with server
  if (handshake(server_sock) == 0)
    printf("Handshake with server was successful\n");
  else
    die("Could not perform handshake with the server\n");

  while (running) {
    socks = master; // Reset socks from master

    // Count sockets with incoming data
    if (state == REGISTERED)
      retval = select(node_listener + 1, &socks, NULL, NULL, NULL);
    else
      retval = select(node_listener + 1, &socks, NULL, NULL, &tv);

    // Socket data handler
    if (retval) {
      // Check standard input
      if (FD_ISSET(STDIN, &socks)) {
        while ((std_input = getchar()) != '\n' && std_input != EOF);
        if (state == REGISTERED){
          printf("Disconnecting...\n");
          state = DEREGISTERING;
        }
        //running = 0;
      }

      // Check if incoming data from server
      if (FD_ISSET(server_sock, &socks)) {
        DHTPacket *pkt = recv_packet(server_sock);
        if (pkt->type == DHT_REGISTER_FAKE_ACK) {
          state = CONNECTED;

        } else if (pkt->type == DHT_REGISTER_BEGIN) {
          node_host = parse_host(pkt->data);
          node_port = parse_port(pkt->data);
          printf("Another node has connected {%s, %d} - sending ack\n",
                  node_host, node_port);
          node_sock = create_socket((char *) node_host, node_port);
          // Perform handshake
          handshake(node_sock);
          int data_len = header_len + tcp_len;
          send_all(node_sock,
                    encode_packet(key, key, DHT_REGISTER_ACK, tcp_len, (void *) tcp_addr),
                    &data_len);
          free(node_host);

        } else if (pkt->type == DHT_DEREGISTER_ACK) {
          // Send DEREGISTER_BEGIN to neighbour nodes
          unsigned char address1[tcp_len + 1];
          unsigned char address2[tcp_len + 1];
          memcpy(address1, pkt->data, tcp_len);
          memcpy(address2, pkt->data + tcp_len, tcp_len);
          address1[tcp_len] = '\0';
          address2[tcp_len] = '\0';
          unsigned char *addresses[2] = {address1, address2};
          for (int i = 0; i < 2; i++) {
            node_host = parse_host(addresses[i]);
            node_port = parse_port(addresses[i]);
            node_sock = create_socket((char *) node_host, node_port);
            handshake(node_sock);
            send_all(node_sock,
                      encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL),
                        &header_len);
            free(node_host);
            close(node_sock);
          }

        } else if (pkt->type == DHT_DEREGISTER_DENY) {
          // Cancel deregistering
          if (state == DEREGISTERING + 1){
            printf("Deregistering denied - press CTRL + C to force disconnect\n");
            state = REGISTERED;
          } else {
            printf("Unexpected packet from server\n");
          }
        }else if (pkt->type == DHT_DEREGISTER_DONE) {
          state++;
        }/* else if (pkt->type == DHT_REGISTER_DONE) {
          // Neighbour ready
        }*/
        destroy_packet(pkt);
      }

      // Check from incoming node connections
      if (FD_ISSET(node_listener, &socks)) {
        node_sock = accept(node_listener, NULL, NULL);
        if (node_sock < 0)
          die("Could not create new node socket");
        else {
          // Perform handshake with node
          handshake_listener(node_sock);
          DHTPacket *pkt = recv_packet(node_sock);

          if (pkt->type == DHT_REGISTER_ACK)
            state++;
          else if (pkt->type == DHT_DEREGISTER_BEGIN)
            // Send acknowledgement of neighbour deregister to server
            send_all(server_sock,
                      encode_packet(pkt->origin, key, DHT_DEREGISTER_DONE, 0, NULL),
                        &header_len);

          destroy_packet(pkt);
        }
        close(node_sock);
      }

    }

    if (!state) {
      // Send REGISTER_BEGIN
      int data_len = header_len + tcp_len;
      send_all(server_sock,
                encode_packet(key, key, DHT_REGISTER_BEGIN, tcp_len, (void *) tcp_addr),
                  &data_len);
      state++;
    }

    if (state == CONNECTED) {
      // Send REGISTER_DONE
      send_all(server_sock,
                encode_packet(key, key, DHT_REGISTER_DONE, 0, NULL),
                  &header_len);
      state = REGISTERED;
      printf("Registered to server - press [enter] to disconnect\n");
    }

    if (state == DEREGISTERING) {
      // Send DEREGISTER_BEGIN
      send_all(server_sock,
                encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL),
                  &header_len);
      state++;
    }
    if (state == DEREGISTERED) {
      // Ok to exit
      printf("Successfully disconnected!\n");
      running = 0;
    }
  }

  // Close socket
  close(server_sock);
  close(node_listener);
  return 0;
}
