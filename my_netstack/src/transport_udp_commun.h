#ifndef _TRANSPORT_UDP_COMMUN_H
#define _TRANSPORT_UDP_COMMUN_H
	#include "netstack.h"

	int udp_sendto(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t* destination_ip, u_int16_t destination_port);
	int udp_recvfrom(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t* source_ip, u_int16_t* source_port);

#endif