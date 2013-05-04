#include "list_node.h"

// Inserts the given DHTPacket into the given list.
void push_to_list(list_node **list, DHTPacket *packet)
{
  list_node *new = malloc(sizeof(list_node));
  if (new == NULL) {
    fprintf(stderr, "Fatal error: Out of memory\n");
    exit(1);
  }
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

