#ifndef DHTPACKET_H_INCLUDED
#define DHTPACKET_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct DHTPacket {
  unsigned char *destination;
  unsigned char *origin;
  unsigned short type;
  unsigned short length;
  unsigned char *data;
} DHTPacket;

// Creates a new DHTPacket by binding (not copying) all the given parameters
DHTPacket *bind_packet(unsigned char *destination, unsigned char *origin,
                        unsigned short type, unsigned short length,
                        unsigned char *data);

// Destroys the given packet
void destroy_packet(DHTPacket *pkt);

// Creates a char array representation of the given parameters
char *encode_packet(unsigned char *destination, unsigned char *origin,
                    unsigned short type, unsigned short length,
                    unsigned char *data);

// Creates a char array representation of the given package
char *serialize_packet(DHTPacket *pkt);

#endif
