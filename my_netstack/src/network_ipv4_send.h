#ifndef _NETWORK_IPV4_SEND_H
#define _NETWORK_IPV4_SEND_H
#include "netstack.h"
#include "network_arp_request.h"
#include "network_arp_reply.h"

void load_ip_header(u_int8_t* ip_buffer, int Network_SendIndex);
void load_ip_data(u_int8_t* ip_buffer, int Network_SendIndex, int len);
DWORD WINAPI read_from_transport(LPVOID pM);
DWORD WINAPI send_to_datalink(LPVOID pM);

#endif