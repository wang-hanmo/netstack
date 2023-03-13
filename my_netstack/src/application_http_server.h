#ifndef _APPLICATION_HTTP_SERVER_H
#define _APPLICATION_HTTP_SERVER_H
	#include "netstack.h"
	#include "transport_tcp_socket.h"
	#include "transport_tcp_commun.h"

	void http_server();
	int load_http_data(char* filename);
	int request_interpreter(char* request, char* filename);

#endif