#ifndef _NETWORK_ICMP_ECHO_H
#define _NETWORK_ICMP_ECHO_H
#include "netstack.h"

//receive
int is_accept_icmp_packet(struct icmp_hdr* icmp_packet);
int network_icmp_recv(u_int8_t* icmp_buffer, u_int8_t* source_ip);
//send
void load_icmp_packet(struct icmp_hdr* icmp_header);
int network_icmp_send(u_int8_t* source_ip);

#endif