#include "netstack.h"
#include "network_arp_request.h"

u_int8_t arp_request_buffer[MAX_IP_PACKAGE_SIZE];

int is_accept_arp_request_packet(struct arp_pkt* arp_packet)
{
	if (ntohs(arp_packet->hardware_type) != ARP_HARDWARE)return 0;
	if (ntohs(arp_packet->protocol_type) != ETHERNET_IP)return 0;
	int i;
	for (i = 0; i < 4; i++)
	{
		if (arp_packet->destination_ip[i] != Local_IP[i])return 0;
	}
	if (ntohs(arp_packet->op_code) == ARP_REQUEST)
	{
		for (i = 0; i < 6; i++)
		{
			if (arp_packet->destination_mac[i] != 0x00)return 0;
		}
	}
	else if (ntohs(arp_packet->op_code) == ARP_REPLY)
	{
		for (i = 0; i < 6; i++)
		{
			if (arp_packet->destination_mac[i] != Local_Mac[i])return 0;
		}
	}

	//add source ip and source mac
	struct arp_node* element;
	if (!is_existed_ip(arp_packet->source_ip))
	{
		element = make_arp_node(arp_packet->source_ip, arp_packet->source_mac, STATIC_STATE);
		insert_arp_node(element);
	}

	return 1;
}

u_int8_t* network_arp_recv_request(u_int8_t* arp_buffer)
{
	if(THREAD_DEBUG)
		printf("\n[接收][数据链路层][write_to_network线程] 成功接收ARP请求报文\n");
	struct arp_pkt* arp_packet = (struct arp_pkt*)arp_buffer;
	if (!is_accept_arp_request_packet(arp_packet))
		return NULL;
	if (ARP_DEBUG) {
		output_arp_protocol(arp_packet);
		output_arp_table();
	}
	/*if arp_request so reply
	else if arp_reply no operation
	*/

	if (ntohs(arp_packet->op_code) == ARP_REQUEST)
	{
		network_arp_send_reply(arp_packet->source_ip, arp_packet->source_mac);
		return NULL;
	}
	else if (ntohs(arp_packet->op_code) == ARP_REPLY)
	{
		return arp_packet->source_mac;
	}
}

void load_arp_request_packet(u_int8_t* destination_ip)
{
	struct arp_pkt* arp_packet = (struct arp_pkt*)(arp_request_buffer);
	arp_packet->hardware_type = htons(ARP_HARDWARE);
	arp_packet->protocol_type = htons(ETHERNET_IP);
	arp_packet->hardware_addr_length = 6;
	arp_packet->protocol_addr_length = 4;
	arp_packet->op_code = htons(ARP_REQUEST);
	int i;
	for (i = 0; i < 6; i++)
	{
		arp_packet->source_mac[i] = Local_Mac[i];
	}
	for (i = 0; i < 4; i++)
	{
		arp_packet->source_ip[i] = Local_IP[i];
	}

	for (i = 0; i < 6; i++)
	{
		arp_packet->destination_mac[i] = 0x00;
	}
	for (i = 0; i < 4; i++)
	{
		arp_packet->destination_ip[i] = destination_ip[i];
	}
}

void network_arp_send_request(u_int8_t* destination_ip, u_int8_t* ethernet_dest_mac)
{
	load_arp_request_packet(destination_ip);
	P(&Network_DataLink_Empty);
	P(&Network_DataLink_Mutex);
	int i;
	for (i = 0; i < sizeof(ip_header); i++)
		Network_DataLink_Pool[Network_DataLink_ReadIndex][i] = arp_request_buffer[i];
	ip_size_of_package[Network_DataLink_ReadIndex] = sizeof(ip_header);
	for (i = 0; i < 6; i++)
		dest_mac[Network_DataLink_ReadIndex][i] = ethernet_dest_mac[i];
	ethernet_type[Network_DataLink_ReadIndex] = ETHERNET_ARP;
	if(THREAD_DEBUG)
		printf("\n[发送][网络层][send_to_datalink线程] 发送ARP请求报文\n");
	Network_DataLink_ReadIndex++;
	V(&Network_DataLink_Mutex);
	V(&Network_DataLink_Full);
}