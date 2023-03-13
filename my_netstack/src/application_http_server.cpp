#include "netstack.h"
#include "application_http_server.h"

char http_server_request[MAX_HTTP_PACKAGE_SIZE] = { 0 };
const char* reply_format =
"HTTP/1.0 %s %s\r\n\
Date:%s\r\n\
Server:Windows 10\r\n\
Last-Modified:%s\r\n\
Content-Length:%d\r\n\
Content-Type:%s\r\n\r\n";

const char* type_format[] = { "text/html", "image/gif", "image/bmp" };
int type_index = 0;

char http_server_reply[MAX_HTTP_PACKAGE_SIZE] = { 0 };
char server_data_buffer[MAX_HTTP_PACKET_SIZE] = { 0 };

void http_server() {
	if (host != SERVER)
		return;
	u_int8_t source_ip[4];
	u_int16_t source_port;
	int len, type;
	char filename[100];
	Socket* sockid = tcp_socket();
	tcp_bind(sockid, (u_int8_t*)Server_IP, Server_Port);
	tcp_listen(sockid, source_ip, &source_port);
	tcp_accept(sockid, source_ip, source_port);
	len = tcp_recv(sockid, (u_int8_t*)http_server_request, MAX_HTTP_PACKAGE_SIZE, 0);
	if(HTTP_DEBUG)
		printf("%s", http_server_request);
	type = request_interpreter(http_server_request, filename);
	if (type == GET) {
		len = load_http_data(filename);
		tcp_send(sockid, (u_int8_t*)http_server_reply, len, 0);
	}
	tcp_recv(sockid, (u_int8_t*)http_server_request, MAX_HTTP_PACKAGE_SIZE, 0);
	tcp_close(sockid);
}
int load_http_data(char* filename) {
	FILE* fp;
	int len, i, j;
	char* p;
	if (filename[0] == '/')
		filename++;
	for (p = filename; *p != '.'; p++);
	p++;
	if (strcmp(p, "html") == 0) {
		fp = fopen(filename, "r");
		type_index = 0;
	}
	else if(strcmp(p,"bmp") == 0) {
		fp = fopen(filename, "rb");
		type_index = 2;
	}
	else {
		fp = fopen(filename, "rb");
		type_index = 1;
	}
	if (fp == NULL) {
		for (i = 0; i < MAX_HTTP_PACKAGE_SIZE; i++)
			http_server_reply[i] = '\0';
		time_t timep;
		time(&timep);
		sprintf(http_server_reply, reply_format, "404", "Not Found", ctime(&timep), ctime(&timep), 0, type_format[type_index]);
		if (HTTP_DEBUG)
			printf("%s", http_server_reply);
		return -1;
	}
	else {
		len = fread(server_data_buffer, 1, MAX_HTTP_PACKET_SIZE, fp);
		fclose(fp);
		for (i = 0; i < MAX_HTTP_PACKAGE_SIZE; i++)
			http_server_reply[i] = '\0';
		time_t timep;
		time(&timep);
		sprintf(http_server_reply, reply_format, "200", "OK", ctime(&timep), ctime(&timep), len, type_format[type_index]);
		if (HTTP_DEBUG)
			printf("%s", http_server_reply);
		i = 0;
		while (http_server_reply[i] != '\0')
			i++;
		for (j = 0; j < len; j++, i++)
			http_server_reply[i] = server_data_buffer[j];
		return i;
	}
	
}
int request_interpreter(char* request, char* filename) {
	char op[10];
	int i, j;
	int type;
	for (i = 0; i < 10; i++) {
		if (request[i] == ' ') {
			op[i] = '\0';
			break;
		}
		op[i] = request[i];
	}
	if (strcmp(op, "Get") == 0)
		type = GET;
	else if (strcmp(op, "Head") == 0)
		type = HEAD;
	else if (strcmp(op, "Delete") == 0)
		type = DELETE;
	else
		type = 0;
	for (i++, j = 0; i < strlen(request); i++, j++) {
		if (request[i] == ' ') {
			filename[j] = '\0';
			break;
		}
		filename[j] = request[i];
	}
	return type;
}