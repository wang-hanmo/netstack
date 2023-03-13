#ifndef _DATALINK_ETHERNET_RECV_H
#define _DATALINK_ETHERNET_RECV_H
#include "netstack.h"
#include "network_arp_request.h"

DWORD WINAPI receive(LPVOID pM);
DWORD WINAPI write_to_network(LPVOID pM);
void ethernet_protocol_packet_callback(u_char* argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content);

#endif