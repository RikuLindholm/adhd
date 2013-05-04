#include "socket.h"

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
  else {
    //printf("Invalid handshake\n");
    ret = 0;
  }
  free(buffer);
  return ret;
}

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
