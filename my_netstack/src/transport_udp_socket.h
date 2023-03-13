#ifndef _TRANSPORT_UDP_SOCKET_H
#define _TRANSPORT_UDP_SOCKET_H
	#include "netstack.h"

	Socket* udp_socket();
	int udp_bind(Socket* sockid, u_int8_t* server_ip, u_int16_t server_port);
	int udp_close(Socket* sockid);
#endif