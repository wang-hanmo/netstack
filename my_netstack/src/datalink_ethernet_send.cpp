#include "netstack.h"
#include "datalink_ethernet_send.h"

int ethernet_size_of_packet[NUM_QUE];
int DataLink_ReadEndFlag = 0;
int DataLink_ReadIndex = 0;
int DataLink_SendIndex = 0;
u_int8_t DataLink_SendPool[NUM_QUE][MAX_ETHERNET_PACKAGE_SIZE] = { NULL };	//缓存队列
int DataLink_Send_Mutex = 1;
int DataLink_Send_Full = 0;
int DataLink_Send_Empty = NUM_QUE;


void load_ethernet_header(u_int8_t* buffer)
{
	struct ethernet_header* hdr = (struct ethernet_header*)buffer;
	int i;
	for (i = 0; i < 6; i++)
		hdr->dest_mac[i] = dest_mac[Network_DataLink_SendIndex][i];
	for (i = 0; i < 6; i++)
		hdr->src_mac[i] = Local_Mac[i];
	hdr->ethernet_type = ethernet_type[Network_DataLink_SendIndex];
}


int load_ethernet_data(u_int8_t* buffer, int Network_DataLink_SendIndex)
{
	int size_of_data = 0;
	int i;
	char tmp[MAX_ETHERNET_PACKAGE_SIZE];
	size_of_data = ip_size_of_package[Network_DataLink_SendIndex];
	for (i = 0; i < size_of_data; i++)
	{
		tmp[i] = Network_DataLink_Pool[Network_DataLink_SendIndex][i];
	}
	u_int32_t crc = calculate_crc((u_int8_t*)tmp, size_of_data);
	for (i = 0; i < size_of_data; i++)
	{
		*(buffer + i) = tmp[i];
	}
	*(u_int32_t*)(buffer + i) = crc;
	return (sizeof(ethernet_header) + size_of_data + 4);
}

DWORD WINAPI read_from_network(LPVOID pM) {
	int flag = 0;
	u_char packet_content[MAX_ETHERNET_PACKAGE_SIZE];
	int Network_DataLink_SendIndex_tmp;
	while (DataLink_ReadIndex < NUM_QUE && (Network_DataLink_SendIndex < Network_DataLink_ReadIndex || Network_DataLink_ReadEndFlag == 0)) {
		P(&Network_DataLink_Full);
		P(&Network_DataLink_Mutex);
		flag = (load_ethernet_data(packet_content + sizeof(ethernet_header), Network_DataLink_SendIndex));
		load_ethernet_header(packet_content);
		Network_DataLink_SendIndex_tmp = Network_DataLink_SendIndex;
		Network_DataLink_SendIndex++;
		V(&Network_DataLink_Mutex);
		V(&Network_DataLink_Empty);
		if(THREAD_DEBUG)
			printf("\n[发送][数据链路层][read_from_network线程] 从上层接收第%d个IP分组，并生成第%d个以太网数据帧\n", Network_DataLink_SendIndex_tmp, DataLink_ReadIndex);
		P(&DataLink_Send_Empty);
		P(&DataLink_Send_Mutex);
		int k;
		for (k = 0; k < flag; k++) {
			DataLink_SendPool[DataLink_ReadIndex][k] = packet_content[k];
		}
		ethernet_size_of_packet[DataLink_ReadIndex] = flag;
		DataLink_ReadIndex++;
		V(&DataLink_Send_Mutex);
		V(&DataLink_Send_Full);
	}
	DataLink_ReadEndFlag = 1;
	if (THREAD_DEBUG)
		printf("\n[发送][数据链路层][read_from_network线程] 已完成！\n");
	system("pause");
	exit(1);
}
DWORD WINAPI send(LPVOID pM) {
	pcap_if_t* d;
	int i = 0;
	char ErrBuf[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs_ex((char*)PCAP_SRC_IF_STRING, NULL, &alldevs, ErrBuf) == -1){
		printf("\n[Ethernet]Error in findalldevs_ex function: %s\n", ErrBuf);
		system("pause");
		exit(1);
	}
	for (d = alldevs, i = 0; i < adapter_id - 1; d = d->next, i++);
	if ((adhandle = pcap_open_live(d->name, 65536, 1, 1000, ErrBuf)) == NULL) {
		printf("\n[Ethernet]Unable to open the adapter.%s is not supported by WinPcap.\n", d->name);
		//pcap_freealldevs(alldevs);
		system("pause");
		exit(1);
	}
	V(&adhandle_available);
	while (DataLink_SendIndex < DataLink_ReadIndex || DataLink_ReadEndFlag == 0){
		P(&DataLink_Send_Full);
		P(&DataLink_Send_Mutex);
		u_int8_t* packet_content = DataLink_SendPool[DataLink_SendIndex];
		struct ethernet_header* ethernet_protocol = (struct ethernet_header*)packet_content;
		u_short ethernet_type = ethernet_protocol->ethernet_type;
		if (ETHERNET_DEBUG) {
			printf("-----Ethernet protocol-------\n");
			printf("Ethernet type: %04x\n", ethernet_type);
			switch (ethernet_type)
			{
			case 0x0800:printf("Upper layer protocol: IPV4\n"); break;
			case 0x0806:printf("Upper layer protocol: ARP\n"); break;
			case 0x8035:printf("Upper layer protocol: RARP\n"); break;
			case 0x814c:printf("Upper layer protocol: SNMP\n"); break;
			case 0x8137:printf("Upper layer protocol: IPX\n"); break;
			case 0x86dd:printf("Upper layer protocol: IPV6\n"); break;
			case 0x880b:printf("Upper layer protocol: PPP\n"); break;
			default:
				break;
			}
			u_char* mac_string = ethernet_protocol->src_mac;
			printf("MAC source address: %02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3),
				*(mac_string + 4), *(mac_string + 5));
			mac_string = ethernet_protocol->dest_mac;
			printf("MAC destination address: %02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2),
				*(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
			printf("Frame Length=%d\n", ethernet_size_of_packet[DataLink_SendIndex]);
		}
		if (IP_DEBUG) {
			if (ethernet_type == 0x0800) {
				struct ip_header* ip_hdr = (struct ip_header*)(packet_content + sizeof(ethernet_header));
				int len = ip_hdr->total_length;
				printf("--------------IP Protocol-------------------\n");
				printf("IP version: %d\n", (ip_hdr->version_hdrlen & 0xf0));
				printf("Type of service: %02x\n", ip_hdr->type_of_service);
				printf("IP packet length: %d\n", ip_hdr->total_length);
				printf("IP identification: %d\n", ip_hdr->id);
				printf("IP fragment & offset: %04x\n", ntohs(ip_hdr->fragment_offset));
				printf("IP time to live: %d\n", ip_hdr->time_to_live);
				printf("Upper protocol type: %02x", ip_hdr->upper_protocol_type);
				u_int8_t upper_protocol_type = ip_hdr->upper_protocol_type;
				switch (upper_protocol_type)
				{
				case IP_TCP:
					printf("   TCP\n");
					//transport_tcp_recv(tcp_buffer);
					break;
				case IP_UDP:
					printf("   UDP\n");
					//transport_udp_recv(udp_buffer);
					break;
				}
				printf("Check sum: %04x\n", ip_hdr->check_sum);
				u_char* ip_string = ip_hdr->source_ip;
				printf("Source IP: %d.%d.%d.%d\n", *ip_string, *(ip_string + 1), *(ip_string + 2), *(ip_string + 3));
				ip_string = ip_hdr->destination_ip;
				printf("Destination IP: %d.%d.%d.%d\n", *ip_string, *(ip_string + 1), *(ip_string + 2), *(ip_string + 3));
				//show the data;
				for (u_int8_t* p = (u_int8_t*)(packet_content + sizeof(ethernet_header) + sizeof(ip_header)); p != (u_int8_t*)(packet_content + sizeof(ethernet_header) + len); p++)
				{
					printf("%c", *p);
				}
			}
		}
		if (pcap_sendpacket(adhandle, (const u_char*)DataLink_SendPool[DataLink_SendIndex], ethernet_size_of_packet[DataLink_SendIndex]) != 0)
			printf("\n[Ethernet]%d.Error sending the packet:%s\n", DataLink_SendIndex, pcap_geterr(adhandle));
		else if(THREAD_DEBUG)
			printf("\n[发送][数据链路层][send线程] 成功发送第%d个以太网数据帧\n", DataLink_SendIndex);
		DataLink_SendIndex++;
		V(&DataLink_Send_Mutex);
		V(&DataLink_Send_Empty);
	}
	if(ETHERNET_DEBUG)
		printf("\n数据帧已经发送完成\n");
	if(THREAD_DEBUG)
		printf("\n[发送][数据链路层][send线程] 已完成！\n");
	system("pause");
	exit(1);
}

