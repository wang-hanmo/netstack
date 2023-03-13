#include "netstack.h"
#include "network_arp_reply.h"

u_int8_t arp_reply_buffer[MAX_IP_PACKAGE_SIZE];

int is_accept_arp_reply_packet(struct arp_pkt* arp_packet)
{
	if (ntohs(arp_packet->hardware_type) != ARP_HARDWARE)return 0;
	if (ntohs(arp_packet->protocol_type) != ETHERNET_IP)return 0;
	if (ntohs(arp_packet->op_code) != ARP_REPLY)return 0;
	int i;
	for (i = 0; i < 6; i++)
	{
		if (arp_packet->destination_mac[i] != Local_Mac[i])return 0;
	}
	for (i = 0; i < 4; i++)
	{
		if (arp_packet->destination_ip[i] != Local_IP[i])return 0;
	}
	for (i = 0; i < 4; i++)
	{
		if (arp_packet->source_ip[i] != Target_IP[i])return 0;
	}

	struct arp_node* element;
	if (!is_existed_ip(arp_packet->source_ip))
	{
		element = make_arp_node(arp_packet->source_ip, arp_packet->source_mac, STATIC_STATE);
		insert_arp_node(element);
	}

	return 1;
}

int network_arp_recv_reply(u_int8_t* arp_buffer, u_int8_t** destination_mac)
{
	struct arp_pkt* arp_packet = (struct arp_pkt*)(arp_buffer);
	if(THREAD_DEBUG)
		printf("\n[发送][网络层][send_to_datalink线程] 成功接收ARP应答报文\n");
	if (is_accept_arp_reply_packet(arp_packet))
	{
		if (ARP_DEBUG) {
			output_arp_protocol(arp_packet);
			output_arp_table();
		}
		*destination_mac = (u_int8_t*)malloc(6 * sizeof(u_int8_t));
		u_int8_t* p = *destination_mac;
		for (int i = 0; i < 6; i++)
			p[i] = arp_packet->source_mac[i];
		return 1;
	}
	return 0;
}

void load_arp_reply_packet(u_int8_t* destination_ip, u_int8_t* ethernet_dest_mac)
{
	struct arp_pkt* arp_packet = (struct arp_pkt*)(arp_reply_buffer);
	arp_packet->hardware_type = htons(ARP_HARDWARE);
	arp_packet->protocol_type = htons(ETHERNET_IP);
	arp_packet->hardware_addr_length = 6;
	arp_packet->protocol_addr_length = 4;
	arp_packet->op_code = htons(ARP_REPLY);
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
		arp_packet->destination_mac[i] = ethernet_dest_mac[i];
	}
	for (i = 0; i < 4; i++)
	{
		arp_packet->destination_ip[i] = destination_ip[i];
	}
}

void network_arp_send_reply(u_int8_t* destination_ip, u_int8_t* ethernet_dest_mac)
{
	struct arp_pkt* arp_packet = (struct arp_pkt*)arp_reply_buffer;
	load_arp_reply_packet(destination_ip, ethernet_dest_mac);
	//send the packet
	P(&Network_DataLink_Empty);
	P(&Network_DataLink_Mutex);
	int i;
	for (i = 0; i < sizeof(ip_header); i++)
		Network_DataLink_Pool[Network_DataLink_ReadIndex][i] = arp_reply_buffer[i];
	ip_size_of_package[Network_DataLink_ReadIndex] = sizeof(ip_header);
	for (i = 0; i < 6; i++)
		dest_mac[Network_DataLink_ReadIndex][i] = ethernet_dest_mac[i];
	ethernet_type[Network_DataLink_ReadIndex] = ETHERNET_ARP;
	if(THREAD_DEBUG)
		printf("\n[接收][数据链路层][write_to_network线程] 发送ARP应答报文\n");
	Network_DataLink_ReadIndex++;
	V(&Network_DataLink_Mutex);
	V(&Network_DataLink_Full);
}