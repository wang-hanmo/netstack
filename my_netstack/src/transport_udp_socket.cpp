#include "netstack.h"
#include "transport_udp_socket.h"

Socket* udp_socket() {
	Socket* sockid = (Socket*)malloc(sizeof(Socket));	//动态分配UDP通信五元组
	int i;
	for (i = 0; i < 4; i++)
		sockid->local_ip[i] = Local_IP[i];
	sockid->local_port = Local_Port;
	for (i = 0; i < 4; i++)
		sockid->target_ip[i] = 0;
	sockid->target_port = -1;
	sockid->sock_type = SOCK_DGRAM;
	return sockid;
}

int udp_bind(Socket* sockid,u_int8_t* server_ip,u_int16_t server_port) {
	int i;
	//绑定服务器地址和端口
	if (host != SERVER)
		return 0;
	for (i = 0; i < 4; i++)
		sockid->local_ip[i] = server_ip[i];
	sockid->local_port = server_port;
	return 1;
}

int udp_close(Socket* sockid) {
	//释放socket
	free(sockid);
	return 1;
}
