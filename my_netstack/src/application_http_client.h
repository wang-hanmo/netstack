#ifndef _APPLICATION_HTTP_CLIENT_H
#define _APPLICATION_HTTP_CLIENT_H
	#include "netstack.h"
	#include "transport_tcp_socket.h"
	#include "transport_tcp_commun.h"

	void http_client(char* url);
	int store_http_data(char* filename, int len);
	int reply_interpreter(char* reply, int len, char** data);
	void http_get(char* filename, char* host); 
	int check_for_more(char* data, int len, char* host, char* url);
#endif