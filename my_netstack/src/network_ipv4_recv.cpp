#include "netstack.h"
#include "network_ipv4_recv.h"

int Network_ReceiveEndFlag = 0;
int Network_WriteIndex = 0;
int Network_ReceiveIndex = 0;
u_int8_t Network_RecvPool[NUM_QUE][MAX_IP_PACKET_SIZE] = { NULL };	//缓存队列
u_int8_t Network_Recv_MF[NUM_QUE];
u_int8_t Network_Recv_Protocol[NUM_QUE];
u_int8_t Network_Recv_Src_IP[NUM_QUE][4];
int Network_Recv_Size[NUM_QUE];
int Network_Recv_Mutex = 1;
int Network_Recv_Full = 0;
int Network_Recv_Empty = NUM_QUE;

u_int16_t ip_id = 0;
int previous = 0, current = 0;

/*
if allow fragment, store to buffer until not allow, then
store to file.
*/



int is_accept_ip_packet(struct ip_header* ip_hdr)
{
	int i, j, flag;
	flag = 0;
	if(IP_DEBUG)
		printf("[IP]pending dest_ip:%d.%d.%d.%d ...\n", ip_hdr->destination_ip[0], ip_hdr->destination_ip[1], ip_hdr->destination_ip[2], ip_hdr->destination_ip[3]);
	for (j = 0; j < 4; j++) {
		if (ip_hdr->destination_ip[j] != Broadcast_IP[j])
			break;
	}
	if (j == 4) {
		flag = 1;//broadcast
		//printf("This is broadcast!\n");
	}
	for (j = 0; j < 4; j++) {
		if (ip_hdr->destination_ip[j] != Local_IP[j])
			break;
	}
	if (j == 4) {
		flag = 2;
	}
	if (flag == 0){
		//printf("[IP]It's not acceptable ip\n");
		return 0;
	}
	for (j = 0; j < 4; j++) {
		if (ip_hdr->source_ip[j] != Local_IP[j])
			break;
	}
	if (j == 4) {
		//printf("[IP]It's from local ip\n");
		return 0;
	}
	u_int16_t check_sum = calculate_check_sum(ip_hdr, 60);
	if (check_sum == 0xffff || check_sum == 0x0000)
	{
		if(IP_DEBUG)
			printf("[IP]No error in ip_header.\n");
	}
	else
	{
		printf("[IP]Error in ip_header\n");
		//network_icmpv4_recv(icmpv4_buffer);
		return 0;
	}
	if (ip_hdr->time_to_live == 0)
	{
		printf("[IP]TTL =0");
		//network_icmpv4_recv(icmpv4_buffer);
		return 0;
	}
	return 1;
}


DWORD WINAPI write_to_transport(LPVOID pM)
{
	int i,cnt = 0;
	u_int8_t tmp[MAX_TCP_PACKAGE_SIZE];
	while (Network_WriteIndex < Network_ReceiveIndex || Network_ReceiveEndFlag == 0) {
		P(&Network_Recv_Full);
		P(&Network_Recv_Mutex);
		for (i = 0; i < Network_Recv_Size[Network_WriteIndex]; i++) {
			tmp[cnt] = Network_RecvPool[Network_WriteIndex][i];
			cnt++;
		}
		if (Network_Recv_MF[Network_WriteIndex] == 0) {
			switch (Network_Recv_Protocol[Network_WriteIndex]) {
				case IP_UDP:
					P(&UDP_Recv_Empty);
					P(&UDP_Recv_Mutex);
					for (i = 0; i < cnt; i++)
						UDP_Recv_Pool[UDP_Recv_ReceiveIndex][i] = tmp[i];
					UDP_Recv_Size[UDP_Recv_ReceiveIndex] = cnt;
					for (i = 0; i < 4; i++)
						UDP_Recv_Src_IP[UDP_Recv_ReceiveIndex][i] = Network_Recv_Src_IP[Network_WriteIndex][i];
					if(THREAD_DEBUG)
						printf("\n[接收][网络层][write_to_transport线程] 接收到第%d组数据，并向上提交第%d个UDP报文\n", Network_WriteIndex, UDP_Recv_ReceiveIndex);
					if(UDP_DEBUG & THREAD_DEBUG)
						printf("UDP length = %d\n", cnt);
					UDP_Recv_ReceiveIndex++;
					V(&UDP_Recv_Mutex);
					V(&UDP_Recv_Full);
					break;
				case IP_TCP:
					P(&TCP_Recv_Empty);
					P(&TCP_Recv_Mutex);
					for (i = 0; i < cnt; i++)
						TCP_Recv_Pool[TCP_Recv_ReceiveIndex][i] = tmp[i];
					TCP_Recv_Size[TCP_Recv_ReceiveIndex] = cnt;
					for (i = 0; i < 4; i++)
						TCP_Recv_Src_IP[TCP_Recv_ReceiveIndex][i] = Network_Recv_Src_IP[Network_WriteIndex][i];
					if (THREAD_DEBUG)
						printf("\n[接收][网络层][write_to_transport线程] 接收到第%d组数据，并向上提交第%d个TCP报文\n", Network_WriteIndex, TCP_Recv_ReceiveIndex);
					if (TCP_DEBUG & THREAD_DEBUG)
						printf("TCP length = %d\n", cnt);
					TCP_Recv_ReceiveIndex++;
					V(&TCP_Recv_Mutex);
					V(&TCP_Recv_Full);
					break;
				default:
					break;
			}
			ip_id++;
			cnt = 0;
			if (ip_id == packages) {
				ip_id--;
				if(THREAD_DEBUG)
					printf("\nrecv_done\n");
				//break;
			}
		}
		Network_WriteIndex++;
		V(&Network_Recv_Mutex);
		V(&Network_Recv_Empty);
	}
	if (THREAD_DEBUG)
		printf("\n[接收][网络层][write_to_transport线程] 已完成！\n");
	system("pause");
	exit(1);
}
DWORD WINAPI receive_from_datalink(LPVOID pM)
{
	u_char ip_buffer[MAX_IP_PACKAGE_SIZE];
	int i;
	struct ip_header* ip_hdr;
	int len;
	u_char* ip_string;
	int DataLink_Network_WriteIndex_tmp;
	int Network_Recv_MF_tmp;
	while ((DataLink_Network_WriteIndex < DataLink_Network_ReceiveIndex || DataLink_Network_ReceiveEndFlag == 0) && Network_ReceiveIndex < NUM_QUE) {
		P(&DataLink_Network_Full);
		P(&DataLink_Network_Mutex);
		for (i = 0; i < ip_size_of_receive[DataLink_Network_WriteIndex]; i++) {
			ip_buffer[i] = DataLink_Network_Pool[DataLink_Network_WriteIndex][i];
		}
		DataLink_Network_WriteIndex_tmp = DataLink_Network_WriteIndex;
		DataLink_Network_WriteIndex++;
		V(&DataLink_Network_Mutex);
		V(&DataLink_Network_Empty);
		ip_hdr = (struct ip_header*)ip_buffer;
		len = ntohs(ip_hdr->total_length);
		//check the valid
		if (!is_accept_ip_packet(ip_hdr)){
			continue;
		}
		u_int16_t fragment;
		fragment = ntohs(ip_hdr->fragment_offset);
		int dural = 0;
		if (previous == 0)
		{
			previous = time(NULL);
		}
		else
		{
			//get current time
			current = time(NULL);
			dural = current - previous;
			if(IP_DEBUG)
				printf("[IP]%d %d\n", current, previous);
			//current time became previous
			previous = current;
		}
		//interval can not larger than 30s
		if (dural >= 30)
		{
			printf("[IP]Time Elapsed.\n");
			continue;
		}

		if (fragment & 0x2000)//true means more fragment
			Network_Recv_MF_tmp = 1;
		else
			Network_Recv_MF_tmp = 0;
		u_int8_t upper_protocol_type = ip_hdr->upper_protocol_type;
		if (IP_DEBUG) {
			printf("--------------IP Protocol-------------------\n");
			printf("IP version: %d\n", (ip_hdr->version_hdrlen & 0xf0));
			printf("Type of service: %02x\n", ip_hdr->type_of_service);
			printf("IP packet length: %d\n", ip_hdr->total_length);
			printf("IP identification: %d\n", ip_hdr->id);
			printf("IP fragment & offset: %04x\n", ntohs(ip_hdr->fragment_offset));
			printf("IP time to live: %d\n", ip_hdr->time_to_live);
			printf("Upper protocol type: %02x", ip_hdr->upper_protocol_type);
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
			case IP_ICMP:
				printf("   ICMP\n");
				break;
			}
			printf("Check sum: %04x\n", ip_hdr->check_sum);
			ip_string = ip_hdr->source_ip;
			printf("Source IP: %d.%d.%d.%d\n", *ip_string, *(ip_string + 1), *(ip_string + 2), *(ip_string + 3));
			ip_string = ip_hdr->destination_ip;
			printf("Destination IP: %d.%d.%d.%d\n", *ip_string, *(ip_string + 1), *(ip_string + 2), *(ip_string + 3));
			//show the data;
			/*for (u_int8_t* p = (u_int8_t*)(ip_buffer + sizeof(ip_header)); p != (u_int8_t*)(ip_buffer + len); p++)
			{
				printf("%c", *p);
			}*/
			printf("\n-----------------End of IP Protocol---------------\n");
		}
		if(THREAD_DEBUG)
			printf("\n[接收][网络层][receive_from_datalink线程] 从下层接收第%d个IP分组，并解析第%d组数据\n", DataLink_Network_WriteIndex_tmp, Network_ReceiveIndex);
		if (upper_protocol_type == IP_ICMP) {//调用ICMP接收函数
			network_icmp_recv((u_int8_t*)(ip_buffer + sizeof(ip_header)), ip_hdr->source_ip);
			continue;
		}
		P(&Network_Recv_Empty);
		P(&Network_Recv_Mutex);
		int k = 0;
		for (u_int8_t* p = (u_int8_t*)(ip_buffer + sizeof(ip_header)); p != (u_int8_t*)(ip_buffer + len); p++) {
			Network_RecvPool[Network_ReceiveIndex][k++] = *p;
		}
		Network_Recv_Size[Network_ReceiveIndex] = k;
		Network_Recv_MF[Network_ReceiveIndex] = Network_Recv_MF_tmp;
		Network_Recv_Protocol[Network_ReceiveIndex] = ip_hdr->upper_protocol_type;
		for(i = 0; i < 4; i++)
			Network_Recv_Src_IP[Network_ReceiveIndex][i] = ip_hdr->source_ip[i];
		Network_ReceiveIndex++;
		V(&Network_Recv_Mutex);
		V(&Network_Recv_Full);
	}
	Network_ReceiveEndFlag = 1;
	if(THREAD_DEBUG)
		printf("\n[接收][网络层][receive_from_datalink线程] 已完成！\n");
	system("pause");
	exit(1);
}
