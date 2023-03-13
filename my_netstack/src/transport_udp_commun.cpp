#include "netstack.h"
#include "transport_udp_commun.h"

u_int8_t udp_send_buffer[MAX_TCP_PACKAGE_SIZE + sizeof(pse_header)] = {0};
u_int8_t udp_recv_buffer[MAX_TCP_PACKAGE_SIZE + sizeof(pse_header)] = { 0 };

int udp_sendto(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t* destination_ip, u_int16_t destination_port) {
	udp_header* udp_hdr = (udp_header*)(udp_send_buffer + sizeof(pse_header));
	pse_header* pse_ptr = (pse_header*)udp_send_buffer;
	u_int8_t* buf_ptr = udp_send_buffer + sizeof(pse_header) + sizeof(udp_header);
	int i;
	//装载UDP首部
	udp_hdr->src_port = sockid->local_port;
	udp_hdr->dest_port = destination_port;
	udp_hdr->length = sizeof(udp_header) + buflen;
	udp_hdr->checksum = 0;
	//装载UDP伪首部
	for (i = 0; i < 4; i++)
		pse_ptr->src_ip[i] = sockid->local_ip[i];
	for (i = 0; i < 4; i++)
		pse_ptr->dest_ip[i] = destination_ip[i];
	pse_ptr->reserve = 0;
	pse_ptr->protocol = 17;
	pse_ptr->length = udp_hdr->length;
	//装载UDP数据
	for (i = 0; i < buflen; i++)
		buf_ptr[i] = buf[i];
	//计算校验和
	udp_hdr->checksum = calculate_check_sum((ip_header*)udp_send_buffer, sizeof(pse_header) + sizeof(udp_header) + buflen);
	P(&Transport_Network_Empty);
	P(&Transport_Network_Mutex);
	for (i = 0; i < udp_hdr->length; i++)
		Transport_Network_Pool[Transport_Network_ReadIndex][i] = *((u_int8_t*)udp_hdr + i);
	tcp_size_of_package[Transport_Network_ReadIndex] = udp_hdr->length;
	for (i = 0; i < 4; i++)
		dest_ip[Transport_Network_ReadIndex][i] = destination_ip[i];
	ip_type[Transport_Network_ReadIndex] = IP_UDP;
	if (THREAD_DEBUG)
		printf("\n[发送][传输层][UDP]成功发送第%d个UDP报文\n",Transport_Network_ReadIndex);
	Transport_Network_ReadIndex++;
	V(&Transport_Network_Mutex);
	V(&Transport_Network_Full);
	return 1; 
}
int udp_recvfrom(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t* source_ip, u_int16_t* source_port) {
	udp_header* udp_hdr = (udp_header*)(udp_recv_buffer + sizeof(pse_header));
	pse_header* pse_ptr = (pse_header*)udp_recv_buffer;
	u_int8_t* buf_ptr = udp_recv_buffer + sizeof(pse_header) + sizeof(udp_header);
	int i, len;
	int UDP_Recv_WriteIndex_tmp;
	u_int16_t checksum;
	P(&UDP_Recv_Full);
	P(&UDP_Recv_Mutex);
	for (i = 0; i < UDP_Recv_Size[UDP_Recv_WriteIndex]; i++)
		*((u_int8_t*)udp_hdr + i) = UDP_Recv_Pool[UDP_Recv_WriteIndex][i];
	for (i = 0; i < 4; i++)
		pse_ptr->src_ip[i] = UDP_Recv_Src_IP[UDP_Recv_WriteIndex][i];
	UDP_Recv_WriteIndex_tmp = UDP_Recv_WriteIndex;
	UDP_Recv_WriteIndex++;
	V(&UDP_Recv_Mutex);
	V(&UDP_Recv_Empty);
	
	if (udp_hdr->dest_port != sockid->local_port) {
		printf("[UDP]Wrong Destination Port! %d-%d\n", udp_hdr->dest_port, sockid->local_port);
		return -1;
	}
	for (i = 0; i < 4; i++)
		pse_ptr->dest_ip[i] = Local_IP[i];
	pse_ptr->reserve = 0;
	pse_ptr->protocol = IP_UDP;
	pse_ptr->length = udp_hdr->length;
	/*
	checksum = udp_hdr->checksum;
	udp_hdr->checksum = 0;
	if (checksum != calculate_check_sum((ip_header*)pse_ptr, sizeof(pse_header) + udp_hdr->length)) {
		printf("[UDP]Wrong Checksum!\n");
			return -1;
	}
	*/
	checksum = calculate_check_sum((ip_header*)pse_ptr, sizeof(pse_header) + udp_hdr->length);
	if (checksum == 0x0000 || checksum == 0xffff) {
		if (UDP_DEBUG)
			printf("[UDP]Right Checksum!\n");
	}
	else {
		printf("[UDP]Wrong Checksum!\n");
		return -1;
	}
	len = udp_hdr->length - sizeof(udp_header);
	if (len > buflen) {
		printf("[UDP]Buffer Overflow!\n");
		return -1;
	}
	if (UDP_DEBUG) {
		printf("--------------UDP Protocol-------------------\n");
		char* ip_string = (char*)pse_ptr->src_ip;
		printf("Source IP: %d.%d.%d.%d\n", *ip_string, *(ip_string + 1), *(ip_string + 2), *(ip_string + 3));
		printf("Source Port:%d\n", udp_hdr->src_port);
		ip_string = (char*)pse_ptr->dest_ip;
		printf("Destination IP: %d.%d.%d.%d\n", *ip_string, *(ip_string + 1), *(ip_string + 2), *(ip_string + 3));
		printf("Destination Port:%d\n", udp_hdr->dest_port);
		printf("Length:%d\n", udp_hdr->length);
		printf("Checksum:%04x\n", checksum);
		//for (i = 0; i < udp_hdr->length - sizeof(udp_header); i++)
		//	printf("%c", buf_ptr[i]);
		printf("\n-----------------End of UDP Protocol---------------\n");
	}
	
	for (i = 0; i < 4; i++)
		source_ip[i] = pse_ptr->src_ip[i];
	*source_port = udp_hdr->src_port;
	for (i = 0; i < len; i++)
		buf[i] = buf_ptr[i];
	if (THREAD_DEBUG)
		printf("\n[接收][传输层][UDP]成功接收第%d个UDP报文\n", UDP_Recv_WriteIndex_tmp);
	
	return len;
}
