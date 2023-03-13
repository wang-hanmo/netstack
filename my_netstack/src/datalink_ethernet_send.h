#ifndef _DATALINK_ETHERNET_SEND_H
#define _DATALINK_ETHERNET_SEND_H
	#include "netstack.h"

	DWORD WINAPI read_from_network(LPVOID pM);
	DWORD WINAPI send(LPVOID pM);
	void load_ethernet_header(u_int8_t* buffer);
	int load_ethernet_data(u_int8_t* buffer, int Network_DataLink_SendIndex);
	
#endif