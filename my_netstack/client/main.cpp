#include "src/netstack.h"
#include "src/datalink_ethernet_send.h"
#include "src/datalink_ethernet_recv.h"
#include "src/network_ipv4_send.h"
#include "src/network_ipv4_recv.h"
#include "src/transport_udp_socket.h"
#include "src/transport_udp_commun.h"
#include "src/transport_tcp_socket.h"
#include "src/transport_tcp_commun.h"
#include "src/application_http_client.h"

int main() {
	generate_crc32_table();
	init_arp_table();
	init_dns();
	//struct arp_node* element = make_arp_node((u_int8_t*)Target_IP, (u_int8_t*)Target_Mac, STATIC_STATE);
	//insert_arp_node(element);
	if(ARP_DEBUG)
		output_arp_table();
	FILE* fp; 
	u_int8_t source_ip[4];
	u_int16_t source_port;
	int i;
	char tmp[MAX_TCP_PACKET_SIZE];
	int size_of_data;

	CreateThread(NULL, 0, read_from_transport, NULL, 0, NULL);
	CreateThread(NULL, 0, send_to_datalink, NULL, 0, NULL);
	CreateThread(NULL, 0, read_from_network, NULL, 0, NULL);
	CreateThread(NULL, 0, send, NULL, 0, NULL);

	CreateThread(NULL, 0, write_to_network, NULL, 0, NULL);
	CreateThread(NULL, 0, receive, NULL, 0, NULL);
	CreateThread(NULL, 0, receive_from_datalink, NULL, 0, NULL);
	CreateThread(NULL, 0, write_to_transport, NULL, 0, NULL);

	/*
	Socket* sockid = udp_socket();
	fp = fopen("data1.txt", "r");
	size_of_data = fread(tmp, 1, MAX_TCP_PACKET_SIZE, fp);
	udp_sendto(sockid, (u_int8_t*)tmp, size_of_data, (u_int8_t*)Target_IP, 8080);
	fclose(fp);
	fp = fopen("data2.txt", "r");
	size_of_data = fread(tmp, 1, MAX_TCP_PACKET_SIZE, fp);
	udp_sendto(sockid, (u_int8_t*)tmp, size_of_data, (u_int8_t*)Target_IP, 8080);
	fclose(fp);
	size_of_data = udp_recvfrom(sockid, (u_int8_t*)tmp, MAX_TCP_PACKET_SIZE, source_ip, &source_port);
	fp = fopen("recv3.txt", "w");
	fwrite(&tmp, 1, size_of_data, fp);
	fclose(fp);
	udp_close(sockid);
	*/
	/*
	Socket* sockid = tcp_socket();
	tcp_connect(sockid, (u_int8_t*)Server_IP, Server_Port);
	fp = fopen("data1.txt", "r");
	size_of_data = fread(tmp, 1, MAX_TCP_PACKET_SIZE, fp);
	tcp_send(sockid, (u_int8_t*)tmp, size_of_data, 0);
	fclose(fp);
	tcp_close(sockid);
	*/
	http_client((char*)"http://www.wanghanmo.com.cn/index.html");
	//while (1);
	system("pause");
	return 0;
}