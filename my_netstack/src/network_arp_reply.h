#ifndef _NETWORK_ARP_REPLY_H
#define _NETWORK_ARP_REPLY_H
#include "netstack.h"

//receive
int is_accept_arp_reply_packet(struct arp_pkt* arp_packet);
int network_arp_recv_reply(u_int8_t* arp_buffer, u_int8_t** destination_mac);
//send
void load_arp_reply_packet(u_int8_t* destination_ip, u_int8_t* ethernet_dest_mac);
void network_arp_send_reply(u_int8_t* destination_ip, u_int8_t* ethernet_dest_mac);

#endif