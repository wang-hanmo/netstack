#include "netstack.h"
#include "network_ipv4_send.h"

int ip_size_of_packet[NUM_QUE];
int Network_ReadEndFlag = 0;
int Network_ReadIndex = 0;
int Network_SendIndex = 0;
u_int8_t Network_SendPool[NUM_QUE][MAX_IP_PACKET_SIZE] = { NULL };	//缓存队列
u_int8_t Network_Send_MF[NUM_QUE];
u_int8_t Network_Send_ID[NUM_QUE];
u_int8_t Network_Send_Dest_IP[NUM_QUE][4];
u_int8_t Network_Send_IP_Type[NUM_QUE];
int Network_Send_Mutex = 1;
int Network_Send_Full = 0;
int Network_Send_Empty = NUM_QUE;


void load_ip_header(u_int8_t* ip_buffer, int Network_SendIndex)
{
	struct ip_header* ip_hdr = (struct ip_header*)ip_buffer;
	//initial the ip header
	ip_hdr->version_hdrlen = 0x4f;//0100 1111 means ip version4 and header length: 60 bytes
	ip_hdr->type_of_service = 0xfe;/*111 1 1110: first 3 bits: priority level,
								   then 1 bit: delay, 1 bit: throughput, 1 bit: reliability
								   1 bit: routing cost, 1 bit: unused
								   */
	ip_hdr->total_length = 0;// wait for data length, 0 for now
	ip_hdr->id = 0;//identification
	ip_hdr->fragment_offset = 0x0000;/*0 0 0 0 00...00: first 3 bits is flag: 1 bit: 0 the last fragment,
								   1 more fragmet. 1 bit: 0 allow fragment, 1 don't fragment. 1 bit: unused
								   the last 12 bits is offset
								   */
	ip_hdr->time_to_live = 64;//default 1000ms
	ip_hdr->upper_protocol_type = Network_Send_IP_Type[Network_SendIndex];//default upper protocol is tcp
	ip_hdr->check_sum = 0;//initial zero
	int i;
	for (i = 0; i < 4; i++)
		ip_hdr->source_ip[i] = Local_IP[i];
	for (i = 0; i < 4; i++)
		ip_hdr->destination_ip[i] = Network_Send_Dest_IP[Network_SendIndex][i];

	//initial check_sum is associate with offset. so in the data we need to calculate check_sum
}

void load_ip_data(u_int8_t* ip_buffer,int Network_SendIndex, int len)
{
	int i = 0;
	char ch;
	while (i < len)
	{
		ch = Network_SendPool[Network_SendIndex][i];
		*(ip_buffer + i) = ch;
		i++;
	}
}
DWORD WINAPI read_from_transport(LPVOID pM) {
	int i,j;
	char tmp[MAX_IP_PACKET_SIZE]={0};
	while ((Transport_Network_SendIndex < Transport_Network_ReadIndex || Transport_Network_ReadEndFlag == 0) && Network_ReadIndex < NUM_QUE) {
		P(&Transport_Network_Full);
		P(&Transport_Network_Mutex);
		for (i = 0; i < tcp_size_of_package[Transport_Network_SendIndex]; i++) {
			if (i % MAX_IP_PACKET_SIZE == 0 && i != 0) {//进行分片交付
				P(&Network_Send_Empty);
				P(&Network_Send_Mutex);
				for (j = 0; j < MAX_IP_PACKET_SIZE; j++)
					Network_SendPool[Network_ReadIndex][j] = tmp[j];
				ip_size_of_packet[Network_ReadIndex] = MAX_IP_PACKET_SIZE;
				Network_Send_MF[Network_ReadIndex] = 1;
				Network_Send_ID[Network_ReadIndex] = Transport_Network_SendIndex;
				for(j = 0; j < 4; j++)
					Network_Send_Dest_IP[Network_ReadIndex][j] = dest_ip[Transport_Network_SendIndex][j];
				Network_Send_IP_Type[Network_ReadIndex] = ip_type[Transport_Network_SendIndex];
				if(THREAD_DEBUG)
					printf("\n[发送][网络层][read_from_transport线程] 读取第%d组数据,ID=%d,Length=%d\n", Network_ReadIndex, Transport_Network_SendIndex, MAX_IP_PACKET_SIZE);
				Network_ReadIndex++;
				V(&Network_Send_Mutex);
				V(&Network_Send_Full);
			}
			tmp[i % MAX_IP_PACKET_SIZE] = Transport_Network_Pool[Transport_Network_SendIndex][i];
		}
		P(&Network_Send_Empty);
		P(&Network_Send_Mutex);
		for (j = 0; j < i % MAX_IP_PACKET_SIZE; j++)//进行剩余分片交付
			Network_SendPool[Network_ReadIndex][j] = tmp[j];
		ip_size_of_packet[Network_ReadIndex] = i % MAX_IP_PACKET_SIZE;
		Network_Send_MF[Network_ReadIndex] = 0;
		Network_Send_ID[Network_ReadIndex] = Transport_Network_SendIndex;
		for (j = 0; j < 4; j++)
			Network_Send_Dest_IP[Network_ReadIndex][j] = dest_ip[Transport_Network_SendIndex][j];
		Network_Send_IP_Type[Network_ReadIndex] = ip_type[Transport_Network_SendIndex];
		if (THREAD_DEBUG)
			printf("\n[发送][网络层][read_from_transport线程] 读取第%d组数据,ID=%d,Length=%d\n", Network_ReadIndex, Transport_Network_SendIndex, i % MAX_IP_PACKET_SIZE + 1);
		Network_ReadIndex++;
		V(&Network_Send_Mutex);
		V(&Network_Send_Full);
		Transport_Network_SendIndex++;
		V(&Transport_Network_Mutex);
		V(&Transport_Network_Empty);
	}
	Network_ReadEndFlag = 1;
	if (THREAD_DEBUG)
		printf("\n[发送][网络层][read_from_transport线程] 已完成！\n");
	system("pause");
	exit(1);
}
DWORD WINAPI send_to_datalink(LPVOID pM) {
	u_int8_t ip_buffer[MAX_IP_PACKAGE_SIZE];
	u_int16_t fragment_offset;
	u_int16_t offset = 0;
	int i;
	int ip_size_of_packet_tmp;
	int Network_SendIndex_tmp;
	while ((Network_SendIndex < Network_ReadIndex || Network_ReadEndFlag == 0) && Network_DataLink_ReadIndex < NUM_QUE) {
		P(&Network_Send_Full);
		P(&Network_Send_Mutex);
		load_ip_header(ip_buffer, Network_SendIndex);
		load_ip_data(ip_buffer + sizeof(ip_header), Network_SendIndex, ip_size_of_packet[Network_SendIndex]);
		struct ip_header* ip_hdr = (struct ip_header*)ip_buffer;
		if (Network_Send_MF[Network_SendIndex] == 1) //不是最后一个分片
			fragment_offset = 0x2000;//16bits
		else //最后一个分片
			fragment_offset = 0x0000;//16bits
		ip_hdr->id = Network_Send_ID[Network_SendIndex];
		ip_size_of_packet_tmp = ip_size_of_packet[Network_SendIndex];
		Network_SendIndex_tmp = Network_SendIndex;
		Network_SendIndex++;
		V(&Network_Send_Mutex);
		V(&Network_Send_Empty);
		fragment_offset |= ((offset / 8) & 0x0fff);
		offset += ip_size_of_packet_tmp;
		ip_hdr->fragment_offset = htons(fragment_offset);
		ip_hdr->total_length = htons(ip_size_of_packet_tmp + sizeof(ip_header));
		ip_hdr->check_sum = calculate_check_sum(ip_hdr, 60);
		//----------------------------------arp---------------------------------
		u_int8_t* destination_mac;
		int lan;
		if (is_same_lan(ip_hdr->source_ip, ip_hdr->destination_ip)) {
			//if dest and src ip are in the same lan
			destination_mac = is_existed_ip(ip_hdr->destination_ip);
			lan = 1;
		}
		else {
			//if dest and src ip are not in the same lan
			destination_mac = is_existed_ip((u_int8_t*)Gate);
			lan = 0;
		}
		if (destination_mac == NULL)
		{
			//check if the target pc and the local host is in the same lan
			if (lan)
				network_arp_send_request(ip_hdr->destination_ip, (u_int8_t*)Broadcast_Mac);
			else
				network_arp_send_request((u_int8_t*)Gate, (u_int8_t*)Broadcast_Mac);
			//wait for replying, get the destination mac
			int len = sizeof(ip_header);
			u_int8_t* pkt_content = (u_int8_t*)malloc(sizeof(u_int8_t)*len);
			while (destination_mac == NULL) {
				if (THREAD_DEBUG)
					printf("\n[发送][网络层][send_to_datalink线程] 等待接收ARP应答报文\n");
				P(&Arp_Reply_Recv_Full);
				P(&Arp_Reply_Recv_Mutex);
				int i;
				for (i = 0; i < len; i++)
					pkt_content[i] = Arp_Reply_Recv_Pool[Arp_Reply_Recv_WriteIndex][i];
				Arp_Reply_Recv_WriteIndex++;
				V(&Arp_Reply_Recv_Mutex);
				V(&Arp_Reply_Recv_Empty);
				network_arp_recv_reply(pkt_content, &destination_mac);
			}
			free(pkt_content);
		}
		P(&Network_DataLink_Empty);
		P(&Network_DataLink_Mutex);
		for (i = 0; i < ip_size_of_packet_tmp + sizeof(ip_header); i++)
			Network_DataLink_Pool[Network_DataLink_ReadIndex][i] = ip_buffer[i];
		ip_size_of_package[Network_DataLink_ReadIndex] = ip_size_of_packet_tmp + sizeof(ip_header);
		for (i = 0; i < 6; i++)
			dest_mac[Network_DataLink_ReadIndex][i] = destination_mac[i];
		//for (i = 0; i < 6; i++)
		//	printf("%02x-",dest_mac[Network_DataLink_ReadIndex][i]);
		//printf("\n");
		ethernet_type[Network_DataLink_ReadIndex] = ETHERNET_IP;
		if (THREAD_DEBUG)
			printf("\n[发送][网络层][send_to_datalink线程] 接收第%d组数据，并向下层发送第%d个IP分组\n", Network_SendIndex_tmp, Network_DataLink_ReadIndex);
		Network_DataLink_ReadIndex++;
		V(&Network_DataLink_Mutex);
		V(&Network_DataLink_Full);
	}
	Network_DataLink_ReadEndFlag = 1;
	if (THREAD_DEBUG)
		printf("\n[发送][网络层][send_to_datalink线程] 已完成！\n");
	system("pause");
	exit(1);
}



