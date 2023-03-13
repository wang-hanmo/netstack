#include "src/netstack.h"
#include "src/datalink_ethernet_recv.h"
#include "src/network_ipv4_recv.h"
#include "src/datalink_ethernet_send.h"
#include "src/network_ipv4_send.h"
#include "src/transport_udp_socket.h"
#include "src/transport_udp_commun.h"
#include "src/transport_tcp_socket.h"
#include "src/transport_tcp_commun.h"
#include "src/application_http_server.h"

int main() {
	generate_crc32_table();
	init_arp_table();
	if(ARP_DEBUG)
		output_arp_table();
	char tmp[MAX_TCP_PACKET_SIZE];
	u_int8_t source_ip[4];
	u_int16_t source_port;
	FILE* fp;
	int len;

	CreateThread(NULL, 0, receive, NULL, 0, NULL);
	CreateThread(NULL, 0, write_to_network, NULL, 0, NULL);
	CreateThread(NULL, 0, receive_from_datalink, NULL, 0, NULL);
	CreateThread(NULL, 0, write_to_transport, NULL, 0, NULL);

	CreateThread(NULL, 0, read_from_transport, NULL, 0, NULL);
	CreateThread(NULL, 0, send_to_datalink, NULL, 0, NULL);
	CreateThread(NULL, 0, read_from_network, NULL, 0, NULL);
	CreateThread(NULL, 0, send, NULL, 0, NULL);

	/*
	Socket* sockid = udp_socket();
	udp_bind(sockid, (u_int8_t*)Server_IP, Server_Port);
	len = udp_recvfrom(sockid, (u_int8_t*)tmp, MAX_TCP_PACKET_SIZE, source_ip, &source_port);
	fp = fopen("recv1.txt", "w");
	fwrite(&tmp, 1, len, fp);
	fclose(fp);
	len = udp_recvfrom(sockid, (u_int8_t*)tmp, MAX_TCP_PACKET_SIZE, source_ip, &source_port);
	fp = fopen("recv2.txt", "w");
	fwrite(&tmp, 1, len, fp);
	fclose(fp);
	fp = fopen("data3.txt", "r");
	len = fread(tmp, 1, MAX_TCP_PACKET_SIZE, fp);
	udp_sendto(sockid, (u_int8_t*)tmp, len, source_ip, source_port);
	fclose(fp);

	udp_close(sockid);
	*/
	/*
	Socket* sockid = tcp_socket();
	tcp_bind(sockid, (u_int8_t*)Server_IP, Server_Port);
	tcp_listen(sockid, source_ip, &source_port);
	tcp_accept(sockid, source_ip, source_port);
	fp = fopen("recv1.txt", "w");
	while(tcp_terminate == 0) {
		len = tcp_recv(sockid, (u_int8_t*)tmp, MAX_TCP_PACKET_SIZE, 0);
		if(tcp_terminate == 0)
			fwrite(&tmp, 1, len, fp);
	}
	fclose(fp);
	tcp_close(sockid);
	*/
	while(1)
		http_server();
	//while (1);
	system("pause");
	return 0;
}