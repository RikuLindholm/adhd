#ifndef LIST_NODE_H_INCLUDED
#define LIST_NODE_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dhtpacket.h"

typedef struct list_node {
    DHTPacket *packet;
    struct list_node *next;
    char selected;
} list_node;

// Inserts the given DHTPacket into the given list.
void push_to_list(list_node **list, DHTPacket *packet);

// Remove the selected nodes from the given list.
void remove_from_list(list_node **list);

// Destroy the given list.
void destroy_list(list_node **list);

#endif
