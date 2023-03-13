#ifndef _NETWORK_IPV4_RECV_H
#define _NETWORK_IPV4_RECV_H
#include "netstack.h"
#include "network_icmp_echo.h"

DWORD WINAPI receive_from_datalink(LPVOID pM);
DWORD WINAPI write_to_transport(LPVOID pM);
int is_accept_ip_packet(struct ip_header* ip_hdr);


#endif