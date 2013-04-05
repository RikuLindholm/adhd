#include "dhtpacket.h"

DHTPacket *create_packet(unsigned char *destination, unsigned char *origin,
                        unsigned short type, unsigned short length,
                        void *data)
{
  DHTPacket *packet = malloc(sizeof(struct DHTPacket));
  packet->destination = destination;
  packet->origin = origin;
  packet->type = type;
  packet->length = length;
  /* MUUTOS
  if (data != NULL)
    packet->data = malloc(sizeof(data));
  */
  packet->data = data;
  return packet;
}

char *encode_packet(unsigned char *destination, unsigned char *origin,
                    unsigned short type, unsigned short length,
                    void *data)
{
  char *result = malloc(44 + length);
  memcpy(result, destination, 20);
  memcpy(result + 20, origin, 20);
  // The following results in big-endian regardless of the machine endianess
  result[40] = (type >> 8) & 0xff;
  result[41] = type & 0xff;
  //result[42] = (length >> 8) & 0xff;
  //result[43] = length & 0xff;
  //memcpy(result + 44, data, length);
  return result;
}

/* Converts the given DHTPacket-structure into a byte array.*/
char *serialize_packet(DHTPacket *packet) {
  return encode_packet(packet->destination, packet->origin,
    packet->type, packet->length, packet->data);
}

/* Converts the given byte array into a DHTPacket-structure.*/
DHTPacket *deserialize_packet(char *data) {
  unsigned char *destination = malloc(20*sizeof(char));
  memcpy(destination, data, 20);
  unsigned char *origin = malloc(20*sizeof(char));
  memcpy(origin, data + 20, 20);
  unsigned short type = ((data[40] & 0xff) << 8) | (data[41] & 0xff);
  unsigned short length = ((data[42] & 0xff) << 8) | (data[43] & 0xff);
  void *payload = malloc(length*sizeof(char));
  memcpy(payload, data + 44, length);
  return create_packet(destination, origin, type, length, payload);
}
