
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DHTPacket {
  unsigned char *destination;   // MUUTOS
  unsigned char *origin;        // MUUTOS
  unsigned short type;
  unsigned short length;
  void *data;
} DHTPacket;

DHTPacket *create_packet(unsigned char *destination, unsigned char *origin,
                                unsigned short type, unsigned short length,
                                void *data) {
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

/* Frees the memory associated for a given DHTPacket.*/
void free_DHTPacket(DHTPacket **packet) {
  free((*packet)->destination);
  free((*packet)->origin);
  free((*packet)->data);
  free(*packet);
}

/* Converts the given DHTPacket-structure into a byte array.*/
char *serialize_packet(DHTPacket *packet) {
  char *data = malloc((packet->length + 44)*sizeof(char));
  memcpy(data, packet->destination, 20);
  memcpy(data + 20, packet->origin, 20);
  // The following results in big-endian regardless of the machine endianess
  data[40] = (packet->type >> 8) & 0xff;
  data[41] = packet->type & 0xff;
  data[42] = (packet->length >> 8) & 0xff;
  data[43] = packet->length & 0xff;
  memcpy(data + 44, packet->data, packet->length);
  return data;
}

/* Converts the given byte array into a DHTPacket-structure.*/
DHTPacket *deserialize_packet(char *data) {
  unsigned char *destination = malloc(20*sizeof(char));
  memcpy(destination, data, 20);
  unsigned char *origin = malloc(20*sizeof(char));
  memcpy(origin, data + 20, 20);
  unsigned short type = (unsigned short)(((data[40] & 0xff) << 8)
                                          | (data[41] & 0xff));
  unsigned short length = (unsigned short)(((data[42] & 0xff) << 8)
                                          | (data[43] & 0xff));
  void *payload = malloc(length*sizeof(char));
  memcpy(payload, data + 44, length);
  return create_packet(destination, origin, type, length, payload);
}

int main() {
  unsigned char destination[20] = "dest_dest_dest_dest";
  unsigned char origin[20] = "orig_orig_orig_orig";
  unsigned short type = (unsigned short)0x4fa2;
  unsigned short length = 8;
  unsigned char payload[8] = "payload";
  DHTPacket *packet = create_packet(destination, origin, type, length, payload);
  char *data = serialize_packet(packet);
  free_DHTPacket(&packet);
  packet = deserialize_packet(data);
  free(data);
  printf("Destination: %s\n", packet->destination);
  printf("Origin: %s\n", packet->origin);
  printf("Type: %x\n", packet->type);
  printf("Length: %x\n", packet->length);
  printf("Payload: %s\n", (char *)packet->data);
  free_DHTPacket(&packet);
  return 0;
}
