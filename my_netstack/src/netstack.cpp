#include "netstack.h"
u_int32_t crc32_table[256];
int packages = 3;
int recv_done = 0;
int tcp_terminate = 0;
tcp_control_block* tcb;
u_int32_t seq_num = 0;
u_int32_t ack_num = 0;
pcap_if_t* alldevs;
pcap_t* adhandle;
int adhandle_available = 0;
dns dns_table[256];
u_int8_t Network_DataLink_Pool[NUM_QUE][MAX_IP_PACKAGE_SIZE] = { NULL };	//Ethernet发送队列
int ip_size_of_package[NUM_QUE];
u_int8_t dest_mac[NUM_QUE][6];
u_int16_t ethernet_type[NUM_QUE];
int Network_DataLink_Mutex = 1;
int Network_DataLink_Full = 0;
int Network_DataLink_Empty = NUM_QUE;
int Network_DataLink_ReadIndex = 0;
int Network_DataLink_SendIndex = 0;
int Network_DataLink_ReadEndFlag = 0;
u_int8_t DataLink_Network_Pool[NUM_QUE][MAX_IP_PACKAGE_SIZE] = { NULL };	//IPv4接收队列
int ip_size_of_receive[NUM_QUE];
int DataLink_Network_Mutex = 1;
int DataLink_Network_Full = 0;
int DataLink_Network_Empty = NUM_QUE;
int DataLink_Network_WriteIndex = 0;
int DataLink_Network_ReceiveIndex = 0;
int DataLink_Network_ReceiveEndFlag = 0;
u_int8_t Arp_Reply_Recv_Pool[NUM_QUE][MAX_IP_PACKAGE_SIZE] = { NULL };		//ARP应答接收队列
int Arp_Reply_Recv_Mutex = 1;
int Arp_Reply_Recv_Full = 0;
int Arp_Reply_Recv_Empty = NUM_QUE;
int Arp_Reply_Recv_WriteIndex = 0;
int Arp_Reply_Recv_ReceiveIndex = 0;
u_int8_t Transport_Network_Pool[NUM_QUE][MAX_TCP_PACKAGE_SIZE] = { NULL };		//IPv4发送队列
int tcp_size_of_package[NUM_QUE];
u_int8_t dest_ip[NUM_QUE][4];
u_int8_t ip_type[NUM_QUE];
int Transport_Network_Mutex = 1;
int Transport_Network_Full = 0;
int Transport_Network_Empty = NUM_QUE;
int Transport_Network_ReadIndex = 0;
int Transport_Network_SendIndex = 0;
int Transport_Network_ReadEndFlag = 0;
u_int8_t UDP_Recv_Pool[NUM_QUE][MAX_TCP_PACKAGE_SIZE];						//UDP接收队列
int UDP_Recv_Mutex = 1;
int UDP_Recv_Full = 0;
int UDP_Recv_Empty = NUM_QUE;
int UDP_Recv_WriteIndex = 0;
int UDP_Recv_ReceiveIndex = 0;
int UDP_Recv_Size[NUM_QUE];
u_int8_t UDP_Recv_Src_IP[NUM_QUE][4];
u_int8_t TCP_Recv_Pool[NUM_QUE][MAX_TCP_PACKAGE_SIZE];						//TCP接收队列
int TCP_Recv_Mutex = 1;
int TCP_Recv_Full = 0;
int TCP_Recv_Empty = NUM_QUE;
int TCP_Recv_WriteIndex = 0;
int TCP_Recv_ReceiveIndex = 0;
int TCP_Recv_Size[NUM_QUE];
u_int8_t TCP_Recv_Src_IP[NUM_QUE][4];

void generate_crc32_table()//generate table
{
	int i, j;
	u_int32_t crc;
	for (i = 0; i < 256; i++)
	{
		crc = i;
		for (j = 0; j < 8; j++)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ 0xEDB88320;
			else
				crc >>= 1;
		}
		crc32_table[i] = crc;
	}
}
u_int32_t calculate_crc(u_int8_t* buffer, int len)
{
	int i;
	u_int32_t crc;
	crc = 0xffffffff;
	for (i = 0; i < len; i++){
		crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ buffer[i]];
	}
	crc ^= 0xffffffff;
	return crc;
}
void P(int* s) {
	while ((*s) <= 0);
	(*s)--;
}
void V(int* s) {
	(*s)++;
}
u_int16_t calculate_check_sum(ip_header* ip_hdr, int len)
{
	int sum = 0, tmp = len;
	u_int16_t* p = (u_int16_t*)ip_hdr;
	while (len > 1)
	{
		sum += *p;
		len -= 2;
		p++;
	}

	//len=1 last one byte
	if (len)
	{
		sum += *((u_int8_t*)ip_hdr + tmp - 1);
	}

	//fold 32 bits to 16 bits
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	return ~sum;
}
int is_same_lan(u_int8_t* local_ip, u_int8_t* destination_ip)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if ((local_ip[i] & Mask[i]) != (destination_ip[i] & Mask[i]))
			return 0;
	}
	return 1;
}

//ARP
arp_table_header arp_table;

struct arp_node* make_arp_node(u_int8_t* ip, u_int8_t* mac, int state)
{
	int i;
	struct arp_node* node = (struct arp_node*)malloc(sizeof(struct arp_node));
	for (i = 0; i < 4; i++)
	{
		node->ip[i] = ip[i];
	}

	for (i = 0; i < 6; i++)
	{
		node->mac[i] = mac[i];
	}
	node->state = state;
	node->next = NULL;
	return node;
}
void init_arp_table()
{
	struct arp_node* node;
	node = make_arp_node((u_int8_t*)Local_IP, (u_int8_t*)Local_Mac, STATIC_STATE);

	arp_table.queue = node;
	arp_table.head = node;
	arp_table.tail = node;
}
void insert_arp_node(struct arp_node* node)
{
	if (!is_existed_ip(node->ip))
	{
		arp_table.tail->next = node;
		arp_table.tail = node;
	}
}
int delete_arp_node(struct arp_node* node)
{
	struct arp_node* pre = arp_table.head;
	struct arp_node* p = pre->next;
	int flag = 1;
	while (p != NULL)
	{
		int i;
		flag = 1;
		for (i = 0; i < 4; i++)
		{
			if (node->ip[i] != p->ip[i])
			{
				flag = 0;
				break;
			}
		}

		for (i = 0; i < 6; i++)
		{
			if (node->mac[i] != p->mac[i])
			{
				flag = 0;
				break;
			}
		}

		if (flag)
		{
			pre->next = p->next;
			free(p);
			break;
		}

		pre = p;
		p = p->next;
	}
	if (flag)
	{
		printf("delete arp node succeed!!!\n");
		return 1;
	}
	else
	{
		printf("Failed delete\n");
		return 0;
	}
}
u_int8_t* is_existed_ip(u_int8_t* destination_ip)
{
	struct arp_node* p = arp_table.head;
	int flag = 1;
	while (p != NULL)
	{
		int i;
		flag = 1;
		for (i = 0; i < 4; i++)
		{
			if (p->ip[i] != destination_ip[i])
			{
				flag = 0;
				break;
			}
		}

		if (flag)
		{
			return p->mac;
		}
		p = p->next;
	}
	return NULL;
}
int update_arp_node(struct arp_node* node)
{
	u_int8_t* mac = is_existed_ip(node->ip);
	if (mac)
	{
		int i;
		for (i = 0; i < 6; i++)
		{
			mac[i] = node->mac[i];
		}
		printf("Update succeed.\n");
		return 1;
	}
	else
	{
		printf("Update failed.\n");
		return 0;
	}
}
void output_arp_table()
{
	struct arp_node* p = arp_table.head;
	printf("\n--------------ARP Cache-------------------\n");
	while (p != NULL)
	{
		int i;
		for (i = 0; i < 4; i++)
		{
			if (i)printf(".");
			printf("%d", p->ip[i]);
		}
		printf("\t");
		for (i = 0; i < 6; i++)
		{
			if (i)printf("-");
			printf("%02x", p->mac[i]);
		}
		printf("\n");

		p = p->next;
	}

}

void output_arp_protocol(struct arp_pkt* arp_packet)
{
	printf("--------ARP Protocol---------\n");
	printf("Hardware Type: %04x\n", ntohs(arp_packet->hardware_type));
	printf("Protocol Type: %04x\n", ntohs(arp_packet->protocol_type));
	printf("Operation Code: %04x\n", ntohs(arp_packet->op_code));
	printf("Source MAC: ");
	int i;
	for (i = 0; i < 6; i++)
	{
		if (i)printf("-");
		printf("%02x", arp_packet->source_mac[i]);
	}
	printf("\nSource IP: ");
	for (i = 0; i < 4; i++)
	{
		if (i)printf(".");
		printf("%d", arp_packet->source_ip[i]);
	}
	printf("\n");
}

int init_dns() {
	strcpy(dns_table[0].host,"www.wanghanmo.com.cn");
	for (int i = 0; i < 4; i++)
		dns_table[0].ip[i] = Server_IP[i];
	dns_table[0].port = Server_Port;
	return 1;
}
int search_dns(char* host, u_int8_t* ip, u_int16_t* port) {
	for (int i = 0; i < 256; i++) {
		if (strcmp(host, dns_table[i].host) == 0) {
			for (int j = 0; j < 4; j++)
				ip[j] = dns_table[i].ip[j];
			*port = dns_table[i].port;
			return 1;
		}
	}
	return 0;
}

int url_interpreter(char* url, char* host, char* filename, u_int8_t* ip, u_int16_t* port) {
	int i, j;
	for (i = 2; i < strlen(url); i++)
		if (url[i - 2] == '/' && url[i - 1] == '/')
			break;
	for (j = 0; j < strlen(url); j++, i++) {
		if (url[i] == '/') {
			host[j] = '\0';
			break;
		}
		host[j] = url[i];
	}
	for (j = 0; j < strlen(url); j++, i++) {
		filename[j] = url[i];
		if (url[i] == '\0')
			break;
	}
	search_dns(host, ip, port);
	return 1;
}