//
//  main.c
//  P2P
//
//  Created by Woochi on 1/4/13.
//  Copyright (c) 2013 Woochi. All rights reserved.
//

#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "list_node.h"
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
#include "sha1.h"
#include "helpers.h"

#define STDIN 0  // file descriptor for standard input

// Parse port from from a given TCP address (port+host)
int parse_port(unsigned char *data)
{
  return (int)(((data[0] & 0xff) << 8) | (data[1] & 0xff));
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
  int ui_listener = create_listen_socket(atoi(argv[5]));
  int node_sock; // Holder for incoming node sockets
  int ui_sock = 0;
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
        // Utility action by providing a string with first character as:
        // 'q': force quit
        first_char = getchar();
        temp_int = first_char;
        while (temp_int != '\n' && temp_int != EOF) temp_int = getchar();
        if (state == REGISTERED){
          if (first_char == 'q') {
            printf("Force disconnect\n");
            running = 0;
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
          int i;
          for (i = 3; i < pkt->length - 4; i++) {
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
          for (i = 0; i < 2; i++) {
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
          // TODO send ACK to UI socket
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
            // The requesting node is not the same as this one
            if (temp_int) {
              // Send the data
              if (ui_sock) {
                printf("Requested data packet found in this node\n");
                // Send the data up to the manager
                temp_pkt = encode_packet(pkt2->destination, key, DHT_SEND_DATA,
                                        pkt2->length, pkt2->data);
                data_len = 0 + pkt2->length;
                put_int(ui_sock, DHT_SEND_DATA);
                put_int(ui_sock, data_len);
                put_bytes(ui_sock, pkt2->data, data_len);
                free(temp_pkt);
              }

            } else {
              // Send no_data
              printf("Requested data packet not found\n");
              if (ui_sock) {
                put_int(ui_sock, DHT_NO_DATA);
              }
            }
          } else {
            // The requesting node is not this one
            printf("Data requested from another node\n");
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
                if (ui_sock) {
                  printf("Requested data packet received\n");
                  // Send the data up to the manager
                  temp_pkt = serialize_packet(pkt);
                  data_len = 0 + pkt->length;
                  send_all(ui_sock, (char *)pkt->data, &data_len);
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
                if (ui_sock) {
                  // Send the data up to the manager
                  temp_pkt = serialize_packet(pkt);
                  data_len = header_len + pkt->length;
                  send_all(ui_sock, temp_pkt, &data_len);
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
          }
        }
        close(node_sock);
      }

      if (FD_ISSET(ui_listener, &socks)) {
        // UI connecting
        printf("UI connected\n");
        ui_sock = accept(ui_listener, NULL, NULL);
        FD_SET(ui_sock, &master); // Add ui socket to master set
        if (greatest_sock < ui_sock)
          greatest_sock = ui_sock;
      }

      if (ui_sock && FD_ISSET(ui_sock, &socks)) {
        printf("Message from UI\n");
        if (is_closed(ui_sock)) {
          printf("UI was closed - disconnecting...\n");
          FD_CLR(ui_sock, &master);
          greatest_sock = ui_listener;
          state = DEREGISTERING;
        } else {
          int type = get_int(ui_sock);
          char *key1 = get_sha1(ui_sock);

          if (type == DHT_PUT_DATA && state == REGISTERED) {
            // Storing data to the DHT
            printf("Storing data...\n");
            unsigned char *data;
            int length = get_int(ui_sock);
            data = get_bytes(ui_sock, length);
            temp_pkt = encode_packet((unsigned char *)key1, key, DHT_PUT_DATA,
                                      length, data);
            data_len = header_len + length;
            send_all(server_sock, temp_pkt, &data_len);
            free(temp_pkt);
            free(data);

          } else if (type == DHT_GET_DATA && state == REGISTERED) {
            printf("Fetching data...\n");
            temp_pkt = encode_packet((unsigned char *)key1, key, DHT_GET_DATA,
                                      tcp_len, tcp_addr);
            data_len = header_len + tcp_len;
            send_all(server_sock, temp_pkt, &data_len);
            free(temp_pkt);

          } else if (type == DHT_DUMP_DATA && state == REGISTERED) {
            printf("Dropping data...\n");
            temp_pkt = encode_packet((unsigned char *)key1, key, DHT_DUMP_DATA,
                                      0, NULL);
            data_len = header_len;
            send_all(server_sock, temp_pkt, &data_len);
            free(temp_pkt);

          }
          free(key1);
        }
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
  if (ui_sock) close(ui_sock);
  return 0;
}
