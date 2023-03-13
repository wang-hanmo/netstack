#ifndef _TRANSPORT_TCP_SOCKET_H
#define _TRANSPORT_TCP_SOCKET_H
	#include "netstack.h"
	#include "transport_tcp_commun.h"

	Socket* tcp_socket();
	int tcp_bind(Socket* sockid, u_int8_t* server_ip, u_int16_t server_port);
	int tcp_connect(Socket* sockid, u_int8_t* server_ip, u_int16_t server_port);
	int tcp_listen(Socket* sockid, u_int8_t* client_ip, u_int16_t* client_port);
	int tcp_accept(Socket* sockid, u_int8_t* client_ip, u_int16_t client_port);
	int tcp_close(Socket* sockid);

#endif