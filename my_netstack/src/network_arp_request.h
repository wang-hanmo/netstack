#ifndef _NETWORK_ARP_REQUEST_H
#define _NETWORK_ARP_REQUEST_H
#include "netstack.h"
#include "network_arp_reply.h"

//receive
int is_accept_arp_request_packet(struct arp_pkt* arp_packet);
u_int8_t* network_arp_recv_request(u_int8_t* arp_buffer);
//send
void load_arp_request_packet(u_int8_t* destination_ip);
void network_arp_send_request(u_int8_t* destination_ip, u_int8_t* ethernet_dest_mac);

#endif