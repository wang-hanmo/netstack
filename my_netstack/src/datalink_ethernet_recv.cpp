#include "netstack.h"
#include "datalink_ethernet_recv.h"

int DataLink_ReceiveIndex = 0;
int DataLink_WriteIndex = 0;
int DataLink_ReceiveEndFlag = 0;
u_int8_t DataLink_RecvPool[NUM_QUE][MAX_ETHERNET_PACKAGE_SIZE] = { NULL };
int ethernet_size_of_receive[NUM_QUE];
int DataLink_Recv_Mutex = 1;
int DataLink_Recv_Full = 0;
int DataLink_Recv_Empty = NUM_QUE;

DWORD WINAPI receive(LPVOID pM)
{
	char error_buffer[PCAP_ERRBUF_SIZE];
	P(&adhandle_available);
	pcap_loop(adhandle, NULL, ethernet_protocol_packet_callback, NULL);
	DataLink_ReceiveEndFlag = 1;
	pcap_freealldevs(alldevs);
	if(THREAD_DEBUG)
		printf("\n[接收][数据链路层][receive线程] 已完成！\n");
	system("pause");
	exit(1);
}

//ethernet protocol analysis
void ethernet_protocol_packet_callback(u_char* argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content)
{
	struct ethernet_header* ethernet_protocol;
	static int packet_number = 1;
	ethernet_protocol = (struct ethernet_header*)packet_content;
	int len = packet_header->len;
	int j;

	////check the mac address
	////if the packet is sended to my pc or broadcast
	int flag = 0;
	for (j = 0; j < 6; j++) {
		if (ethernet_protocol->dest_mac[j] != Broadcast_Mac[j])
			break;
	}
	if (j == 6) {
		flag = 1;
		//printf("This is broadcast!\n");
	}
	for (j = 0; j < 6; j++) {
		if (ethernet_protocol->dest_mac[j] != Local_Mac[j])
			break;
	}
	if (j == 6) {
		flag = 2;
	}
	if (flag == 0)
		return;
	for (j = 0; j < 6; j++) {
		if (ethernet_protocol->src_mac[j] != Local_Mac[j])
			break;
	}
	if (j == 6)
		return;

	//see if the data is changed or not
	u_int32_t crc = calculate_crc((u_int8_t*)(packet_content + sizeof(ethernet_header)), len - 4 - sizeof(ethernet_header));
	if (crc != *((u_int32_t*)(packet_content + len - 4)))
	{
		printf("[Ethernet]The data has been changed.(crc32 wrong)\n");
		return;
	}
	if (ETHERNET_DEBUG) {
		printf("----------------------------\n");
		printf("capture %d packet\n", packet_number);
		printf("capture time: %d\n", packet_header->ts.tv_sec);
		printf("packet length: %d\n", packet_header->len);
		printf("----------------------\n");
	}
	int k = 0;
	P(&DataLink_Recv_Empty);
	P(&DataLink_Recv_Mutex);
	for (u_int8_t* p = (u_int8_t*)(packet_content); p != (u_int8_t*)(packet_content + packet_header->len - 4); p++)
	{
		DataLink_RecvPool[DataLink_ReceiveIndex][k++] = *p;
	}
	ethernet_size_of_receive[DataLink_ReceiveIndex] = packet_header->len;
	if (THREAD_DEBUG)
		printf("\n[接收][数据链路层][receive线程] 接收第%d个以太网数据帧\n", DataLink_ReceiveIndex);
	DataLink_ReceiveIndex++;
	V(&DataLink_Recv_Mutex);
	V(&DataLink_Recv_Full);
	packet_number++;
}

DWORD WINAPI write_to_network(LPVOID pM)
{
	u_short ethernet_type;
	struct ethernet_header* ethernet_protocol;
	u_char* mac_string;
	u_char packet_content[MAX_ETHERNET_PACKAGE_SIZE];
	int i;
	int ethernet_size_of_receive_tmp;
	int DataLink_WriteIndex_tmp;
	while ((DataLink_WriteIndex < DataLink_ReceiveIndex || DataLink_ReceiveEndFlag == 0) && DataLink_Network_ReceiveIndex < NUM_QUE) {
		P(&DataLink_Recv_Full);
		P(&DataLink_Recv_Mutex);
		for (i = 0; i < ethernet_size_of_receive[DataLink_WriteIndex]; i++) {
			packet_content[i] = DataLink_RecvPool[DataLink_WriteIndex][i];
		}
		DataLink_WriteIndex_tmp = DataLink_WriteIndex;
		ethernet_size_of_receive_tmp = ethernet_size_of_receive[DataLink_WriteIndex];
		DataLink_WriteIndex++;
		V(&DataLink_Recv_Mutex);
		V(&DataLink_Recv_Empty);
		ethernet_protocol = (struct ethernet_header*)packet_content;
		ethernet_type = ethernet_protocol->ethernet_type;
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
			mac_string = ethernet_protocol->src_mac;
			printf("MAC source address: %02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3),
				*(mac_string + 4), *(mac_string + 5));
			mac_string = ethernet_protocol->dest_mac;
			printf("MAC destination address: %02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2),
				*(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
			//show the data;
			/*for (u_int8_t* p = (u_int8_t*)(packet_content + sizeof(ethernet_header)); p != (u_int8_t*)(packet_content + ethernet_size_of_receive_tmp - 4); p++)
			{
				printf("%c", *p);
			}*/
			printf("\n");

			printf("----------------------\n");
		}
		if (ethernet_type == ETHERNET_ARP)
		{
			struct arp_pkt* arp_packet = (struct arp_pkt*)(packet_content + sizeof(ethernet_header));
			if (arp_packet->op_code == htons(ARP_REQUEST)) {
				network_arp_recv_request(packet_content + sizeof(ethernet_header));
			}
			else if (arp_packet->op_code == htons(ARP_REPLY)) {
				P(&Arp_Reply_Recv_Empty);
				P(&Arp_Reply_Recv_Mutex);
				int i;
				for (i = 0; i < sizeof(ip_header); i++)
					Arp_Reply_Recv_Pool[Arp_Reply_Recv_ReceiveIndex][i] = *(packet_content + sizeof(ethernet_header) + i);
				Arp_Reply_Recv_ReceiveIndex++;
				V(&Arp_Reply_Recv_Mutex);
				V(&Arp_Reply_Recv_Full);
			}
			continue;
		}

		
		if (THREAD_DEBUG)
			printf("\n[接收][数据链路层][write_to_network线程] 解析第%d个以太网数据帧，并向上层发送第%d个IP分组\n", DataLink_WriteIndex_tmp, DataLink_Network_ReceiveIndex);
		P(&DataLink_Network_Empty);
		P(&DataLink_Network_Mutex);
		int k = 0;
		for (u_int8_t* p = (u_int8_t*)(packet_content + sizeof(ethernet_header)); p != (u_int8_t*)(packet_content + ethernet_size_of_receive_tmp - 4); p++) {
			DataLink_Network_Pool[DataLink_Network_ReceiveIndex][k++] = *p;
		}
		ip_size_of_receive[DataLink_Network_ReceiveIndex] = ethernet_size_of_receive_tmp - 4 - sizeof(ethernet_header);
		DataLink_Network_ReceiveIndex++;
		V(&DataLink_Network_Mutex);
		V(&DataLink_Network_Full);
	}
	DataLink_Network_ReceiveEndFlag = 1;
	if (THREAD_DEBUG)
		printf("\n[接收][数据链路层][write_to_network线程] 已完成！\n");
	system("pause");
	exit(1);
}
