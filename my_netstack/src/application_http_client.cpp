#include "netstack.h"
#include "application_http_client.h"

char http_client_request[MAX_HTTP_PACKAGE_SIZE] = { 0 };
const char* request_format =
"%s %s HTTP/1.0\r\n\
Host:%s\r\n\
Connection:close\r\n\
User-agent:WIN/4.0\r\n\
Accept:text/html,image/gif,image/bmp\r\n\
Accept-language:cn\r\n\r\n";

const char* Not_Found_404 = "<HTML><HEAD><TITLE>404 NOT FOUND</TITLE></HEAD></HTML>";

char http_client_reply[MAX_HTTP_PACKAGE_SIZE] = { 0 };
char* client_data_buffer;

void http_client(char* url) {
	if (host != CLIENT)
		return;
	char host[100];
	char filename[100];
	u_int8_t ip[4];
	u_int16_t port;
	int len;
	url_interpreter(url, host, filename, ip, &port);
	//printf("host:%s", host);
	//printf("filename:%s", filename);
	Socket* sockid = tcp_socket();
	tcp_connect(sockid, ip, port);
	http_get(filename, host);
	tcp_send(sockid, (u_int8_t*)http_client_request, strlen(http_client_request), 0);
	len = tcp_recv(sockid, (u_int8_t*)http_client_reply, MAX_HTTP_PACKAGE_SIZE, 0);
	len = reply_interpreter(http_client_reply, len, &client_data_buffer);
	if (len == 0) {
		printf("404 Not Found!\n");
		FILE* fp;
		char* p = filename;
		if (filename[0] == '/')
			p++;
		fp = fopen(p, "w");
		fwrite(Not_Found_404, 1, strlen(Not_Found_404), fp);
		fclose(fp);
		return;
	}
	store_http_data(filename, len);
	tcp_close(sockid);
	char tmp[100];
	while(check_for_more(client_data_buffer, len, host, tmp))
		http_client(tmp);

}
int store_http_data(char* filename, int len) {
	FILE* fp;
	char* p;
	if (filename[0] == '/')
		filename++;
	for (p = filename; *p != '.'; p++);
	p++;
	if (strcmp(p, "html") == 0)
		fp = fopen(filename, "w");
	else
		fp = fopen(filename, "wb");
	fwrite(client_data_buffer, 1, len, fp);
	fclose(fp);
	return 1;
}

int reply_interpreter(char* reply, int len, char** data) {
	int i;
	if (reply[9] == '4' && reply[10] == '0' && reply[11] == '4')
		return 0;
	for (i = 0; i < len; i++) {
		if (i < 4) {
			if (HTTP_DEBUG)
				printf("%c", reply[i]);
		}
		else if (reply[i - 1] == '\n' && reply[i - 2] == '\r' && reply[i - 3] == '\n' && reply[i - 4] == '\r') {
			break;
		}
		else {
			if (HTTP_DEBUG) {
				printf("%c", reply[i]);
			}
		}
				
	}
	*data = &reply[i];
	return (len - i);
}
void http_get(char* filename, char* host) {
	sprintf(http_client_request, request_format, "Get", filename, host);
	if (HTTP_DEBUG)
		printf("%s", http_client_request);
}

int check_for_more(char* data, int len, char* host, char* url) {
	int i, j;
	strcpy(url, "http://");
	strcat(url, host);
	for (i = 0; i < len; i++) {
		if (i < 4)
			continue;
		else if (data[i - 1] == '=' && data[i - 2] == 'C' && data[i - 3] == 'R' && data[i - 4] == 'S')
				break;
	}
	if (i == len)
		return 0;
	for (j = 0; url[j] != '\0'; j++);
	url[j++] = '/';
	for (; ; j++, i++) {
		url[j] = data[i];
		if (data[i] == '>') {
			url[j] = '\0';
			break;
		}
	}
	return 1;
}