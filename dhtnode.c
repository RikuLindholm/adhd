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
#include "socket.h"

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
// Handshake for manager
static const unsigned short DHT_MANAGE_SHAKE = 0x516a;

typedef struct list_node {
    DHTPacket *packet;
    struct list_node *next;
    char selected;
} list_node;

// Utility method for printing error messages and exiting
void die(char *reason)
{
  fprintf(stderr, "Fatal error: %s\n", reason);
  exit(1);
}

// Inserts the given DHTPacket into the given list.
void push_to_list(list_node **list, DHTPacket *packet)
{
  list_node *new = malloc(sizeof(list_node));
  if (new == NULL) die("Out of memory");
  new->packet = packet;
  new->selected = 0;
  if (list) new->next = *list;
  else new->next = NULL;
  *list = new;
}

// Remove the selected nodes from the given list.
void remove_from_list(list_node **list)
{
  if (list == NULL) return;
  list_node *cur = *list;
  list_node *prev = NULL;
  list_node *next;
  while (cur) {
    if (cur->selected) {
      next = cur->next;
      destroy_packet(cur->packet);
      free(cur);
      cur = next;
    } else {
      if (prev)
        prev->next = cur;
      else
        *list = cur;
      prev = cur;
      cur = cur->next;
    }
  }
  if (prev) prev->next = NULL;
  else *list = NULL;
}

// Destroy the given list.
void destroy_list(list_node **list)
{
  if (list == NULL) return;
  list_node *cur = *list;
  list_node *next;
  while (cur) {
    next = cur->next;
    destroy_packet(cur->packet);
    free(cur);
    cur = next;
  }
  *list = NULL;
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

  DHTPacket *pkt = bind_packet(destination, origin, type, length, payload);
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
// Return:  1 if client hanshake was received
//          2 if manager handshake was received
//          0 otherwise
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
  int ret;
  if (s_shake == DHT_CLIENT_SHAKE)
    ret = 1;
  else if (s_shake == DHT_MANAGE_SHAKE)
    ret = 2;
  else {
    //printf("Invalid handshake\n");
    ret = 0;
  }
  free(buffer);
  return ret;
}

// Parse port from from a given TCP address (port+host)
int parse_port(unsigned char *data)
{
  return (int)(((data[0] & 0xff) << 8) | (data[1] & 0xff));
}

// Places the modular difference between keys a and b into array result.
// All arrays must be of length SHA1_DIGEST_LENGTH.
// Retur: 1 if a >= b, 0 otherwise
int difference(unsigned char *result, unsigned char *a, unsigned char *b)
{
  // Check if a is bigger than b. If not -> switch
  int a_bigger = 1;
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (b[i] > a[i]) {
      unsigned char *temp = a;
      a = b;
      b = temp;
      a_bigger = 0;
      break;
    } else if (b[i] < a[i])
      break;
  }
  // Perform subtraction a - b
  short carry = 0;
  short v1;
  for (int i = SHA1_DIGEST_LENGTH - 1; i >= 0; i--) {
    v1 = (short)a[i] - carry - (short)b[i];
    carry = 0;
    if (v1 < 0){
      carry = 1;
      v1 += 0x10000;
    }
    result[i] = v1 & 0xff;
  }
  // Check if the complement distance is closer i.e. smaller than 0x80..0
  unsigned short limit = 0x80;
  for (int j = 0; j < SHA1_DIGEST_LENGTH; j++) {
    if ( result[j] > limit ) {
      // Complement is closer
      for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
        result[i] = ~result[i];
      }
      for (int i = SHA1_DIGEST_LENGTH - 1; i >= 0; i--) {
        if (result[i] != 0xff) {
          result[i] += 1;
          break;
        }
        result[i] = 0;
      }
      break;
    } else if (result[j] < limit)
      break;
    if (!j) limit = 0;
  }
  return a_bigger;
}

// Check which of keys a and b is closer to the target key. If they are equally
// close, the next key from the target to the incrementing direction is
// considered closer. All arrays must be of length SHA1_DIGEST_LENGTH.
// Return: 1 if a is closer, 0 if b is closer, -1 if a and b are equal.
int find_closer_key(unsigned char *target, unsigned char *a, unsigned char *b)
{
  // Calculate differences
  unsigned char dif_a[SHA1_DIGEST_LENGTH];
  int a_bigger_t = difference(dif_a, a, target);
  unsigned char dif_b[SHA1_DIGEST_LENGTH];
  int b_bigger_t = difference(dif_b, b, target);
  // Check if one is closer
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (dif_b[i] > dif_a[i])
      return 1;
    if (dif_b[i] < dif_a[i])
      return 0;
  }
  // Distances are equal. Checking which one is in incremental direction.
  // N.B. in this point a and b cant be equal to the target.
  if (a_bigger_t && !b_bigger_t)
    return 1;
  if (!a_bigger_t && b_bigger_t)
    return 0;
  // Check if a or b is bigger
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (b[i] > a[i]) {
      return 1;
    } else if (b[i] < a[i])
      return 0;
  }
  // a and b are equal
  return -1;
}


// Utility function for checking if two SHA1 keys are equal
// Return: 1 if equal, 0 if not equal
int compare_keys(unsigned char *key1, unsigned char *key2)
{
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (key1[i] != key2[i])
      return 0;
  }
  return 1;
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
  unsigned short tcp_len = strlen(argv[3]) + 3;
  unsigned char tcp_addr[tcp_len];
  unsigned short port = atoi(argv[4]);
  tcp_addr[0] = (port >> 8) & 0xff;     // In network byte order
  tcp_addr[1] = port & 0xff;
  memcpy(tcp_addr + 2, argv[3], strlen(argv[3]));
  tcp_addr[tcp_len - 1] = '\0';

  // Construct SHA1 key
  unsigned char key[SHA1_DIGEST_LENGTH];
  sha1(tcp_addr, (unsigned int) tcp_len, key);

  // Create sockets
  int server_sock = create_socket((char *) argv[1], atoi(argv[2]));
  int node_listener = create_listen_socket(atoi(argv[4]));
  int node_sock; // Holder for incoming node sockets
  int ui_listener = create_listen_socket(52000);
  int manager_sock = 0;
  int greatest_sock = ui_listener;

  //printf("Setting timeout values");
  // Set read timeout to zero
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&master); //Reset master socket set
  FD_ZERO(&socks); //Reset temp socket set
	FD_SET(STDIN, &master); // Add standard input to master set
	FD_SET(server_sock, &master); // Add server sock master set
	FD_SET(node_listener, &master); // Add server listener to master set
	FD_SET(ui_listener, &master); // Add server listener to master set

  // Perform initial handshake with server
  if (handshake(server_sock) == 0)
    printf("Handshake with server was successful\nJoining to the network\n");
  else
    die("Could not perform handshake with the server\n");
  
  char *temp_pkt;
  list_node *datalist = NULL;
  list_node *cur_node;
  unsigned char *compare_key1, *compare_key2, *temp_char_array;
  int data_len;
  int temp_int, first_char = 0;
  DHTPacket *pkt, *pkt2;
  
  // Test keys
  // #1: SHA1 from "50000 localhost": b9e2f030f9685d8f504931f60bf08a5c1e382545
  // #2: SHA1 from "50001 localhost": c70984fa4af2a16a9a5489ac1055fa3791355268
  // #3: SHA1 from "50002 localhost": 52f3d5c7fa258e04ac21bf51928b7cd9b90f9370
  // Near #1 but closer to #2 than #3
  unsigned char test_key1[20] =
                {0xb9,0xe2,0xf0,0x30,0xf9,0x68,0x5d,0x8f,0x50,0x49,
                 0x31,0xf6,0x0b,0xf0,0x8a,0x5c,0x1e,0x38,0x25,0x46};
  // Near #1 but closer to #3 than #2
  unsigned char test_key2[20] =
                {0x8a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  // Near #2 but closer to #1 than #3
  unsigned char test_key3[20] =
                {0xc7,0x09,0x84,0xfa,0x4a,0xf2,0xa1,0x6a,0x9a,0x54,
                 0x89,0xac,0x10,0x55,0xfa,0x37,0x91,0x35,0x52,0x67};
  // Near #2 but closer to #3 than #1
  unsigned char test_key4[20] =
                {0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  // Near #3 but closer to #1 than #2
  unsigned char test_key5[20] =
                {0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  // Near #3 but closer to #2 than #1
  unsigned char test_key6[20] =
                {0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  unsigned char *test_keys[6] = {test_key1, test_key2, test_key3,
                                    test_key4, test_key5, test_key6};  
  unsigned char *test_key;
  unsigned char test_data[] = "test_data_x";
  
  
  while (running) {
    socks = master; // Reset socks from master

    // Count sockets with incoming data
    if (state == REGISTERED)
      retval = select(greatest_sock + 1, &socks, NULL, NULL, NULL);
    else
      retval = select(greatest_sock + 1, &socks, NULL, NULL, &tv);

    // Socket data handler
    if (retval) {
      // Check standard input
      if (FD_ISSET(STDIN, &socks)) {
        // Utility actions by providing a string with first character as:
        // 'q': force quit
        // 'l': list the current data of the node
        // '1'-'6': store test_data 1-6 given above
        // 'a'-'f': get and print the test_data 1-6
        // 'A'-'F': drop the test_data 1-6
        first_char = getchar();
        temp_int = first_char;
        while (temp_int != '\n' && temp_int != EOF) temp_int = getchar();
        if (state == REGISTERED){
          if (first_char == 'q') {
            printf("Force disconnect\n");
            running = 0;
          } else if (first_char == 'l') {
            if (datalist != NULL){
              printf("List of data in this node:\n");
              cur_node = datalist;
              while (cur_node) {
                printf("| %.*s\n", cur_node->packet->length,
                        cur_node->packet->data);
                cur_node = cur_node->next;
              }
              printf("----------------\n");
            } else printf("No data in this node\n");
          } else if ('1' <= first_char && first_char <= '6') {
            // Store data
            data_len = strlen((char *) test_data) + 1;
            test_data[data_len - 2] = first_char;
            printf("Storing data: \"%s\"\n", test_data);
            //sha1(test_data, (unsigned int) data_len, test_key);
            test_key = test_keys[first_char - 49];
            temp_pkt = encode_packet(test_key, key, DHT_PUT_DATA,
                                      data_len, test_data);
            data_len = header_len + data_len;
            send_all(server_sock, temp_pkt, &data_len);
            free(temp_pkt);
            
          } else if ('a' <= first_char && first_char <= 'f') {
            // Get and print
            data_len = strlen((char *) test_data) + 1;
            test_data[data_len - 2] = first_char - 48;
            printf("Requesting for \"%s\"\n", test_data);
            //sha1(test_data, (unsigned int) data_len, test_key);
            test_key = test_keys[first_char - 97];
            temp_pkt = encode_packet(test_key, key, DHT_GET_DATA,
                                      tcp_len, tcp_addr);
            data_len = header_len + tcp_len;
            send_all(server_sock, temp_pkt, &data_len);
            free(temp_pkt);
            
          } else if ('A' <= first_char && first_char <= 'F') {
            // Drop
            data_len = strlen((char *) test_data) + 1;
            test_data[data_len - 2] = first_char - 16;
            printf("Requesting \"%s\" to be dropped\n", test_data);
            //sha1(test_data, (unsigned int) data_len, test_key);
            test_key = test_keys[first_char - 65];
            temp_pkt = encode_packet(test_key, key, DHT_DUMP_DATA, 0, NULL);
            data_len = header_len;
            send_all(server_sock, temp_pkt, &data_len);
            free(temp_pkt);
            
          } else {
            printf("Disconnecting...\n");
            state = DEREGISTERING;
          }
        }
      }

      // Check if incoming data from server
      if (FD_ISSET(server_sock, &socks)) {
        pkt = recv_packet(server_sock);
        if (pkt->type == DHT_REGISTER_FAKE_ACK) {
          state = CONNECTED;
          destroy_packet(pkt);

        } else if (pkt->type == DHT_REGISTER_BEGIN) {
          node_host = pkt->data + 2;
          node_port = parse_port(pkt->data);
          printf("New neighbour {%s, %d}", node_host, node_port);
          node_sock = create_socket((char *) node_host, node_port);
          // Perform handshake
          handshake(node_sock);
          // Send the data that is closer to the new node
          temp_int = 0;
          if (datalist != NULL){
            compare_key1 = pkt->destination;
            cur_node = datalist;
            while (cur_node) {
              compare_key2 = cur_node->packet->destination;
              if (cur_node->selected == 0 &&
                    find_closer_key(compare_key2, compare_key1, key) == 1) {
                // packet in cur_node is closer to the new node
                temp_int++;
                temp_pkt = encode_packet(compare_key2, key, DHT_TRANSFER_DATA,
                                          cur_node->packet->length,
                                          cur_node->packet->data);
                data_len = header_len + cur_node->packet->length;
                send_all(node_sock, temp_pkt, &data_len);
                free(temp_pkt);
                cur_node->selected = 1; // Mark to be deleted when all ready
              }
              cur_node = cur_node->next;
            }
          }
          if (temp_int)
            printf(", %d data packets send\n", temp_int);
          else printf(", no data send\n");
          // Tell that all was send
          temp_pkt = encode_packet(key, key, DHT_REGISTER_ACK, 0, NULL);
          data_len = header_len;
          send_all(node_sock, temp_pkt, &data_len);
          free(temp_pkt);
          destroy_packet(pkt);
        
        } else if (pkt->type == DHT_REGISTER_DONE) {
          // Drop the not needed data chuncks that was send for the new node
          remove_from_list(&datalist);
          destroy_packet(pkt);
        
        } else if (pkt->type == DHT_DEREGISTER_ACK) {
          // Deregistering accepted
          // Separate the addresses
          temp_char_array = pkt->data;
          temp_int = 0;
          for (int i = 3; i < pkt->length - 4; i++) {
            if (temp_char_array[i] == '\0') {
              temp_int = i + 1;
              break;
            }
          }
          if (temp_int == 0)
            die("Corrupted addresses received from the server");
          unsigned int len_addr1 = temp_int - 3;
          unsigned int len_addr2 = pkt->length - temp_int - 3;
          unsigned char address1[len_addr1 + 1];
          unsigned char address2[len_addr2 + 1];
          memcpy(address1, temp_char_array + 2, len_addr1);
          memcpy(address2, temp_char_array + 2 + temp_int, len_addr2);
          address1[len_addr1] = '\0';
          address2[len_addr2] = '\0';
          unsigned char neigh1_key[SHA1_DIGEST_LENGTH];
          unsigned char neigh2_key[SHA1_DIGEST_LENGTH];
          sha1(temp_char_array, len_addr1 + 3, neigh1_key);
          sha1(temp_char_array + temp_int, len_addr2 + 3, neigh2_key);          
          // Mark the ones that are for neighbour1 as selected
          // If both neighbours are the same one, leave everything unselected
          if (datalist != NULL && strcmp((char *) temp_char_array,
               (char *) (temp_char_array + temp_int)) != 0){
            cur_node = datalist;
            while (cur_node) {
              compare_key2 = cur_node->packet->destination;
              if (find_closer_key(compare_key2, neigh1_key, neigh2_key) == 1)
                cur_node->selected = 1; // This one for the neighbour1
              cur_node = cur_node->next;
            }
          }
          // For each neighbour, send the respective data and confirm
          unsigned char *addresses[2] = {address1, address2};
          for (int i = 0; i < 2; i++) {
            node_host = addresses[i];
            node_port = parse_port(temp_char_array + i*temp_int);
            node_sock = create_socket((char *) node_host, node_port);
            handshake(node_sock);
            // Send the right data packets
            if (datalist != NULL){
              cur_node = datalist;
              while (cur_node) {
                if ((cur_node->selected == 1 && i == 0)
                      || (cur_node->selected == 0 && i == 1)) {
                  temp_pkt = encode_packet(cur_node->packet->destination, key,
                                            DHT_TRANSFER_DATA,
                                            cur_node->packet->length,
                                            cur_node->packet->data);
                  data_len = header_len + cur_node->packet->length;
                  send_all(node_sock, temp_pkt, &data_len);
                  free(temp_pkt);
                  cur_node->selected = 1; // Mark to be deleted when all ready
                }
                cur_node = cur_node->next;
              }
            }
            // Send DEREGISTER_BEGIN to neighbour
            temp_pkt = encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL);
            data_len = header_len;
            send_all(node_sock, temp_pkt, &data_len);
            close(node_sock);
            free(temp_pkt);
          }
          destroy_packet(pkt);

        } else if (pkt->type == DHT_DEREGISTER_DENY) {
          // Cancel deregistering
          if (state == DEREGISTERING + 1){
            printf("Deregistering denied - write 'q' and press ENTER to force disconnect\n");
            state = REGISTERED;
          } else {
            printf("Unexpected packet from server\n");
          }
          destroy_packet(pkt);
        
        } else if (pkt->type == DHT_DEREGISTER_DONE) {
          state++;
          destroy_packet(pkt);
        
        } else if (pkt->type == DHT_PUT_DATA) {
          // Receive data (N.B. no two identical keys should be added)
          printf("New data packet received\n");
          push_to_list(&datalist, pkt);
          temp_pkt = encode_packet(pkt->destination, pkt->origin,
                                    DHT_PUT_DATA_ACK, 0, NULL);
          data_len = header_len;
          send_all(server_sock, temp_pkt, &data_len);
          free(temp_pkt);
        
        } else if (pkt->type == DHT_PUT_DATA_ACK) {
          // Data stored succesfull to the dht
          printf("Data stored succesfully\n");
          destroy_packet(pkt);
          
        } else if (pkt->type == DHT_GET_DATA) {
          // Requesting data
          // Search for the requested key
          temp_int = 0;  // if zero, the key was not found
          if (datalist != NULL){
            compare_key1 = pkt->destination;
            cur_node = datalist;
            while (cur_node) {
              if (compare_keys(compare_key1, cur_node->packet->destination)) {
                temp_int = 1;
                pkt2 = cur_node->packet;
                break;
              }
              cur_node = cur_node->next;
            }
          }
          // Checking that the request was not from this node
          if (strcmp((char *)tcp_addr, (char *)pkt->data) == 0) {
            // The requesting node is the same as this one
            if (temp_int) {
              // Send the data
              if (manager_sock) {
                printf("Requested data packet found in this node\n");
                // Send the data up to the manager
                temp_pkt = encode_packet(pkt2->destination, key, DHT_SEND_DATA,
                                        pkt2->length, pkt2->data);
                data_len = header_len + pkt2->length;
                send_all(manager_sock, temp_pkt, &data_len);
                free(temp_pkt);
              } else if ('a' <= first_char && first_char <= 'f') {
                // Test utility printing
                printf("Requested data received: \"%s\"\n", pkt2->data);
                first_char = 0;
              }
            } else {
              // Send no_data
              printf("Requested data packet not found\n");
              if (manager_sock) {
                // Send the data up to the manager
                temp_pkt = encode_packet(key, key, DHT_NO_DATA, 0, NULL);
                data_len = header_len;
                send_all(manager_sock, temp_pkt, &data_len);
                free(temp_pkt);
              }
            }
          } else {
            // The requesting node is not this one
            printf("Data requested from this node\n");
            node_host = pkt->data + 2;
            node_port = parse_port(pkt->data);
            node_sock = create_socket((char *) node_host, node_port);
            // Perform handshake
            handshake(node_sock);
            if (temp_int) {
              // Send the data
              temp_pkt = encode_packet(pkt2->destination, key, DHT_SEND_DATA,
                                        pkt2->length, pkt2->data);
              data_len = header_len + pkt2->length;
              send_all(node_sock, temp_pkt, &data_len);
              free(temp_pkt);
            } else {
              // Send no_data
              temp_pkt = encode_packet(key, key, DHT_NO_DATA, 0, NULL);
              data_len = header_len;
              send_all(node_sock, temp_pkt, &data_len);
              free(temp_pkt);
            }
          }
          pkt2 = NULL;
          destroy_packet(pkt);
        
        } else if (pkt->type == DHT_DUMP_DATA) {
          // Dump data
          if (datalist != NULL){
            compare_key1 = pkt->destination;
            cur_node = datalist;
            while (cur_node) {
              if (compare_keys(compare_key1, cur_node->packet->destination)) {
                // Dump node found
                cur_node->selected = 1;
                remove_from_list(&datalist);
                printf("Data dropped in this node\n");
                break;
              }
              cur_node = cur_node->next;
            }
          }
          temp_pkt = encode_packet(pkt->destination, pkt->origin,
                                    DHT_DUMP_DATA_ACK, 0, NULL);
          data_len = header_len;
          send_all(server_sock, temp_pkt, &data_len);
          free(temp_pkt);
          destroy_packet(pkt);
        
        } else if (pkt->type == DHT_DUMP_DATA_ACK) {
          // Data dropped succesfull
          printf("Requested data dropped succesfully\n");
          destroy_packet(pkt);
        
        } else {
          // Other packet received
          destroy_packet(pkt);
        }
      }

      // Check from incoming node connections
      if (FD_ISSET(node_listener, &socks)) {
        node_sock = accept(node_listener, NULL, NULL);
        if (node_sock < 0)
          die("Could not create new node socket");
        else {
          // Perform handshake with node
          retval = handshake_listener(node_sock);
          if (retval == 1) {
            // Loop until no incoming data transfer packets
            temp_int = 0;
            while (1) {
              pkt = recv_packet(node_sock);
              
              if (pkt->type == DHT_TRANSFER_DATA) {
                temp_int++;
                push_to_list(&datalist, pkt);
              
              } else if (pkt->type == DHT_REGISTER_ACK) {
                printf("%d data packets received from a neighbour\n", temp_int);
                state++;
                destroy_packet(pkt);
                break;
              
              } else if (pkt->type == DHT_DEREGISTER_BEGIN) {
                // Send acknowledgement of neighbour deregister to server
                printf("Neighbour deregistering");
                if (temp_int)
                  printf(", %d data packets received\n", temp_int);
                else
                  printf("\n");
                temp_pkt = encode_packet(pkt->origin, key,
                                          DHT_DEREGISTER_DONE, 0, NULL);
                data_len = header_len;
                send_all(server_sock, temp_pkt, &data_len);
                free(temp_pkt);
                destroy_packet(pkt);
                break;
              
              } else if (pkt->type == DHT_SEND_DATA) {
                // Receiving a requested data packet
                if (manager_sock) {
                  printf("Requested data packet received\n");
                  // Send the data up to the manager
                  temp_pkt = serialize_packet(pkt);
                  data_len = header_len + pkt->length;
                  send_all(manager_sock, temp_pkt, &data_len);
                  free(temp_pkt);
                } else if ('a' <= first_char && first_char <= 'f') {
                  // Test utility printing
                  printf("Requested data received: \"%s\"\n", pkt->data);
                  first_char = 0;
                }
                destroy_packet(pkt);
                break;
              
              } else if (pkt->type == DHT_NO_DATA) {
                // Requested data packet not found
                printf("Requested data packet not found\n");
                if (manager_sock) {
                  // Send the data up to the manager
                  temp_pkt = serialize_packet(pkt);
                  data_len = header_len + pkt->length;
                  send_all(manager_sock, temp_pkt, &data_len);
                  free(temp_pkt);
                }
                destroy_packet(pkt);
                break;
              
              } else {
                // Unsupported packet
                destroy_packet(pkt);
                break;
              }
            }
          } else if (retval == 2 && manager_sock == 0) {
            // Manager connecting
            printf("Manager connected");
            manager_sock = node_sock;
            if (greatest_sock < manager_sock)
              greatest_sock = manager_sock;
            FD_SET(manager_sock, &master); // Add server listener to master set
          }
        }
        close(node_sock);
      }

      if (FD_ISSET(ui_listener, &socks)) {
        printf("Message from UI\n");
        int sock = accept(ui_listener, NULL, NULL);
        int type = getInt(sock);
        char *key = getSha1(sock);

        printf("Received type: %d\n", type);
        printf("Received key: %s\n", key);

        if (type == DHT_PUT_DATA && state == REGISTERED) {
          // Storing data to the DHT
          printf("Storing data...\n");
          int length = getInt(sock);
          unsigned char *data = getBlock(sock, length);
          printf("Received length: %d\n", length);
          printf("Received data");
          temp_pkt = encode_packet(pkt->destination, (unsigned char *)key, DHT_PUT_DATA,
                                    length, data);
          data_len = header_len + length;
          send_all(server_sock, temp_pkt, &data_len);
          free(temp_pkt);
        
        } else if (type == DHT_GET_DATA && state == REGISTERED) {
          printf("Fetching data...\n");
          temp_pkt = encode_packet(pkt->destination, (unsigned char *)key, DHT_GET_DATA,
                                    tcp_len, tcp_addr);
          data_len = header_len + tcp_len;
          send_all(server_sock, temp_pkt, &data_len);
          free(temp_pkt);
        } else if (type == DHT_DUMP_DATA && state == REGISTERED) {
          printf("Dropping data...\n");
          temp_pkt = encode_packet(pkt->destination, (unsigned char *)key, DHT_DUMP_DATA,
                                    0, NULL);
          data_len = header_len;
          send_all(server_sock, temp_pkt, &data_len);
          free(temp_pkt);
        }

        close(sock);
      }
    }

    if (!state) {
      // Send REGISTER_BEGIN
      data_len = header_len + tcp_len;
      temp_pkt = encode_packet(key, key, DHT_REGISTER_BEGIN,
                                tcp_len, tcp_addr);
      send_all(server_sock, temp_pkt, &data_len);
      free(temp_pkt);
      state++;
    }

    if (state == CONNECTED) {
      // Send REGISTER_DONE
      temp_pkt = encode_packet(key, key, DHT_REGISTER_DONE, 0, NULL);
      data_len = header_len;
      send_all(server_sock, temp_pkt, &data_len);
      free(temp_pkt);
      state = REGISTERED;
      printf("Registered to server - press [enter] to disconnect\n");
    }

    if (state == DEREGISTERING) {
      // Send DEREGISTER_BEGIN
      temp_pkt = encode_packet(key, key, DHT_DEREGISTER_BEGIN, 0, NULL);
      data_len = header_len;
      send_all(server_sock, temp_pkt, &data_len);
      free(temp_pkt);
      state++;
    }
    if (state == DEREGISTERED) {
      // Ok to exit
      printf("Successfully disconnected!\n");
      running = 0;
    }
  }
  
  // Destroy data
  destroy_list(&datalist);
  
  // Close sockets
  close(server_sock);
  close(node_listener);
  if (manager_sock) close(manager_sock);
  return 0;
}
