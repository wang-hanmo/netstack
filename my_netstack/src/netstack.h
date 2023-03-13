#ifndef _NETSTACK_H
#define _NETSTACK_H
	//basic configuration
	#include<stdio.h>
	#include<stdlib.h>
	#include<string.h>
	#include<math.h>
	#include<time.h>
	#define HAVE_REMOTE
	#include<pcap.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
	#pragma comment(lib,"Packet.lib")
	#pragma warning(disable:4996)
	//localhost message
	#define CLIENT 0
	#define SERVER 1
	#include "../localhost.h"

	//marco difinition
	#define MAX_ETHERNET_PACKET_SIZE 1496
	#define MAX_ETHERNET_PACKAGE_SIZE 1514
	#define MAX_IP_PACKET_SIZE 1400
	#define MAX_IP_PACKAGE_SIZE 1460
	#define MAX_TCP_PACKET_SIZE 65476
	#define MAX_TCP_PACKAGE_SIZE 65536
	#define MAX_HTTP_PACKET_SIZE 60000
	#define MAX_HTTP_PACKAGE_SIZE 65000
	#define NUM_QUE 100
	//protocol type
	#define ETHERNET_IP 0x0800
	#define ETHERNET_ARP 0x0806
	#define ETHERNET_RARP 0X8035
	#define IP_ICMP 1
	#define IP_IGMP 2
	#define IP_TCP 6
	#define IP_UDP 17
	//ARP
	#define ARP_HARDWARE 0X0001
	#define ARP_REQUEST 0X0001
	#define ARP_REPLY 0X0002
	#define STATIC_STATE 0
	#define DYNAMIC_STATE 1
	#define LOGGING_STATE 2
	//ICMP
	#define ICMP_ECHO_REQUEST 8
	#define ICMP_ECHO_REPLY 0
	//socket
	#define SOCK_DGRAM 17
	#define SOCK_STREAM 6
	//TCP flags
	#define URG 32
	#define ACK 16
	#define PSH 8
	#define RST 4
	#define SYN 2
	#define FIN 1
	//HTTP
	#define GET 1
	#define HEAD 2
	#define DELETE 3
	//DEBUG
	#define THREAD_DEBUG 0
	#define ETHERNET_DEBUG 0
	#define IP_DEBUG 0
	#define ARP_DEBUG 0
	#define ICMP_DEBUG 0
	#define UDP_DEBUG 0
	#define TCP_DEBUG 1
	#define HTTP_DEBUG 1

	//struct header
	struct ethernet_header {
		u_int8_t dest_mac[6];
		u_int8_t src_mac[6];
		u_int16_t ethernet_type;
	};
	struct ip_header{
		u_int8_t version_hdrlen;// default IP version: ipv4, header_length: 60bytes
		u_int8_t type_of_service;//
		u_int16_t total_length;//
		u_int16_t id;			//identification
		u_int16_t fragment_offset;//packet maybe need to be fraged. 
		u_int8_t time_to_live;
		u_int8_t upper_protocol_type;
		u_int16_t check_sum;
		u_int8_t source_ip[4];   //this is a structure equval to u_int32_t, but canbe used in windows socket api
		u_int8_t destination_ip[4];
		u_int8_t optional[40];//40 bytes is optional
	};
	struct Socket {
		u_int8_t local_ip[4];
		u_int16_t local_port;
		u_int8_t target_ip[4];
		u_int16_t target_port;
		u_int8_t sock_type;
	};
	struct udp_header {
		u_int16_t src_port;
		u_int16_t dest_port;
		u_int16_t length;
		u_int16_t checksum;
	};
	struct tcp_header {
		u_int16_t src_port;
		u_int16_t dest_port;
		u_int32_t sequence;
		u_int32_t acknowledge;
		u_int8_t header_length;
		u_int8_t flags;
		u_int16_t winsize;
		u_int16_t checksum;
		u_int16_t urgent_offset;
		u_int8_t optional[40];
	};
	struct pse_header {
		u_int8_t src_ip[4];
		u_int8_t dest_ip[4];
		u_int8_t reserve;
		u_int8_t protocol;
		u_int16_t length;
	};
	struct tcp_control_block {
		u_int32_t client_initial_sequence;
		u_int32_t server_initial_sequence;
		u_int16_t send_window_size;
		u_int16_t send_cache_size;
		u_int16_t recv_window_size;
		u_int16_t recv_cache_size;
		u_int16_t client_mss;
		u_int16_t server_mss;
		u_int16_t communication_mss;
	};
	//DNS
	struct dns {
		char host[100];
		u_int8_t ip[4];
		u_int16_t port;
	}; 
	//ARP
	struct arp_node
	{
		u_int8_t ip[4];
		u_int8_t mac[6];
		u_int8_t state;
		struct arp_node* next;
	};
	struct arp_table_header
	{
		arp_node* queue;
		arp_node* head;
		arp_node* tail;
	};
	struct arp_pkt
	{
		u_int16_t hardware_type;
		u_int16_t protocol_type;
		u_int8_t hardware_addr_length;
		u_int8_t protocol_addr_length;
		u_int16_t op_code;
		u_int8_t source_mac[6];
		u_int8_t source_ip[4];
		u_int8_t destination_mac[6]; //request the mac addr
		u_int8_t destination_ip[4];
	};
	//ICMP
	struct icmp_hdr
	{
		u_int8_t type;
		u_int8_t code;
		u_int16_t checksum;
		u_int16_t id;
		u_int16_t seq;
		u_int32_t timestamp;
	};

	//public pool
	extern u_int8_t Network_DataLink_Pool[NUM_QUE][MAX_IP_PACKAGE_SIZE];
	extern int ip_size_of_package[NUM_QUE];
	extern u_int8_t dest_mac[NUM_QUE][6];
	extern u_int16_t ethernet_type[NUM_QUE];
	extern int Network_DataLink_Mutex;
	extern int Network_DataLink_Full;
	extern int Network_DataLink_Empty;
	extern int Network_DataLink_ReadIndex;
	extern int Network_DataLink_SendIndex;
	extern int Network_DataLink_ReadEndFlag;
	extern u_int8_t DataLink_Network_Pool[NUM_QUE][MAX_IP_PACKAGE_SIZE];
	extern int ip_size_of_receive[NUM_QUE];
	extern int DataLink_Network_Mutex;
	extern int DataLink_Network_Full;
	extern int DataLink_Network_Empty;
	extern int DataLink_Network_WriteIndex;
	extern int DataLink_Network_ReceiveIndex;
	extern int DataLink_Network_ReceiveEndFlag;
	extern u_int8_t Arp_Reply_Recv_Pool[NUM_QUE][MAX_IP_PACKAGE_SIZE];
	extern int Arp_Reply_Recv_Mutex;
	extern int Arp_Reply_Recv_Full;
	extern int Arp_Reply_Recv_Empty;
	extern int Arp_Reply_Recv_WriteIndex;
	extern int Arp_Reply_Recv_ReceiveIndex;
	extern u_int8_t Transport_Network_Pool[NUM_QUE][MAX_TCP_PACKAGE_SIZE];
	extern int tcp_size_of_package[NUM_QUE];
	extern u_int8_t dest_ip[NUM_QUE][4];
	extern u_int8_t ip_type[NUM_QUE];
	extern int Transport_Network_Mutex;
	extern int Transport_Network_Full;
	extern int Transport_Network_Empty;
	extern int Transport_Network_ReadIndex;
	extern int Transport_Network_SendIndex; 
	extern int Transport_Network_ReadEndFlag;
	extern u_int8_t UDP_Recv_Pool[NUM_QUE][MAX_TCP_PACKAGE_SIZE];
	extern int UDP_Recv_Mutex;
	extern int UDP_Recv_Full;
	extern int UDP_Recv_Empty;
	extern int UDP_Recv_WriteIndex;
	extern int UDP_Recv_ReceiveIndex;
	extern int UDP_Recv_Size[NUM_QUE];
	extern u_int8_t UDP_Recv_Src_IP[NUM_QUE][4];
	extern u_int8_t TCP_Recv_Pool[NUM_QUE][MAX_TCP_PACKAGE_SIZE];
	extern int TCP_Recv_Mutex;
	extern int TCP_Recv_Full;
	extern int TCP_Recv_Empty;
	extern int TCP_Recv_WriteIndex;
	extern int TCP_Recv_ReceiveIndex;
	extern int TCP_Recv_Size[NUM_QUE];
	extern u_int8_t TCP_Recv_Src_IP[NUM_QUE][4];

	const char rand_data[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789./";
	//host message
	const u_int8_t Broadcast_Mac[6] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
	const u_int8_t Broadcast_IP[4] = { 0xff, 0xff, 0xff, 0xff };
	const int adapter_id = 1;
	const u_int16_t Local_Port = 54321;
	const u_int8_t Server_IP[4] = { 10,13,80,37 };
	const u_int16_t Server_Port = 8080;

	//general varibles
	extern int packages;
	extern int recv_done;
	//tcp varibles
	extern int tcp_terminate;
	extern tcp_control_block* tcb;
	extern u_int32_t seq_num;
	extern u_int32_t ack_num;
	//pcap handle
	extern pcap_if_t* alldevs;
	extern pcap_t* adhandle;
	extern int adhandle_available;
	//table
	extern u_int32_t crc32_table[256];
	extern dns dns_table[256];
	extern arp_table_header arp_table;


	//general function
	void P(int* s);
	void V(int* s);
	void generate_crc32_table();
	u_int32_t calculate_crc(u_int8_t* buffer, int len);
	u_int16_t calculate_check_sum(ip_header* ip_hdr, int len);
	int is_same_lan(u_int8_t* local_ip, u_int8_t* destination_ip);
	//ARP
	void init_arp_table();
	struct arp_node* make_arp_node(u_int8_t* ip, u_int8_t* mac, int state);
	void insert_arp_node(struct arp_node* node);
	int delete_arp_node(struct arp_node* node);
	int update_arp_node(struct arp_node* node);
	u_int8_t* is_existed_ip(u_int8_t* destination_ip);
	void output_arp_table();
	void output_arp_protocol(struct arp_pkt* arp_packet);
	//DNS
	int init_dns();
	int search_dns(char* host, u_int8_t* ip, u_int16_t* port);
	int url_interpreter(char* url, char* host, char* filename, u_int8_t* ip, u_int16_t* port);


#endif