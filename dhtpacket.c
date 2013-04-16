#include "dhtpacket.h"

DHTPacket *bind_packet(unsigned char *destination, unsigned char *origin,
                                unsigned short type, unsigned short length,
                                unsigned char *data)
{
  DHTPacket *packet = malloc(sizeof(struct DHTPacket));
  packet->destination = destination;
  packet->origin = origin;
  packet->type = type;
  packet->length = length;
  packet->data = data;
  return packet;
}

void destroy_packet(DHTPacket *pkt)
{
  if (pkt == NULL) return;
  free(pkt->destination);
  free(pkt->origin);
  if (pkt->data != NULL)
    free(pkt->data);
  free(pkt);
}

char *encode_packet(unsigned char *destination, unsigned char *origin,
                    unsigned short type, unsigned short length,
                    unsigned char *data)
{
  char *result = malloc(44 + length);
  memcpy(result, destination, 20);
  memcpy(result + 20, origin, 20);
  // The following results in big-endian regardless of the machine endianess
  result[40] = (type >> 8) & 0xff;
  result[41] = type & 0xff;
  result[42] = (length >> 8) & 0xff;
  result[43] = length & 0xff;
  memcpy(result + 44, data, length);
  return result;
}

/* Converts the given DHTPacket-structure into a byte array.*/
char *serialize_packet(DHTPacket *packet)
{
  return encode_packet(packet->destination, packet->origin,
    packet->type, packet->length, packet->data);
}

