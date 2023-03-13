#ifndef _TRANSPORT_TCP_COMMUN_H
#define _TRANSPORT_TCP_COMMUN_H
	#include "netstack.h"

	int tcp_send(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t flags);
	int tcp_recv(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t flags);
#endif