#ifndef DHTPACKET_H_INCLUDED
#define DHTPACKET_H_INCLUDED

#include <stdlib.h>
#include <string.h>

typedef struct DHTPacket {
  unsigned char *destination;
  unsigned char *origin;
  unsigned short type;
  unsigned short length;
  unsigned char *data;
} DHTPacket;

DHTPacket *create_packet(unsigned char *destination, unsigned char *origin,
                        unsigned short type, unsigned short length,
                        unsigned char *data);

char *encode_packet(unsigned char *destination, unsigned char *origin,
                    unsigned short type, unsigned short length,
                    unsigned char *data);

char *serialize_packet(DHTPacket *pkt);

DHTPacket *deserialize_packet(char *);

#endif
