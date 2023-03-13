#include "netstack.h"
#include "network_icmp_echo.h"

u_int8_t icmp_buffer[MAX_IP_PACKAGE_SIZE];

int is_accept_icmp_packet(struct icmp_hdr* icmp_packet)
{
	if (icmp_packet->type != ICMP_ECHO_REQUEST) return 0;
	u_int16_t check_sum = calculate_check_sum((ip_header*)icmp_packet, 12);
	if (check_sum == 0xffff || check_sum == 0x0000)
		return 1;
	else
		printf("[ICMP]error in icmp_header.\n");
	return 0;
}

int network_icmp_recv(u_int8_t* icmp_buffer, u_int8_t* source_ip)
{
	if(THREAD_DEBUG)
		printf("\n[接收][网络层][receive_from_datalink线程] 成功接收ICMP报文\n");
	struct icmp_hdr* icmp_packet = (struct icmp_hdr*)icmp_buffer;
	if (!is_accept_icmp_packet(icmp_packet))
		return 0;

	network_icmp_send(source_ip);
	return 1;
}

void load_icmp_packet(struct icmp_hdr* icmp_header) {
	icmp_header->type = ICMP_ECHO_REPLY;
	icmp_header->code = 0;
	icmp_header->id = GetCurrentProcessId();
	icmp_header->checksum = 0;
	icmp_header->seq = 0;
	icmp_header->timestamp = GetTickCount();
	char* data_buf = (char*)icmp_header + sizeof(icmp_hdr);
	for (int i = 0; i < 64; i++) {
		data_buf[i] = rand_data[i];
	}
}

int network_icmp_send(u_int8_t* source_ip) {
	struct icmp_hdr* icmp_header = (struct icmp_hdr*)icmp_buffer;
	int i;
	load_icmp_packet(icmp_header);
	P(&Transport_Network_Empty);
	P(&Transport_Network_Mutex);
	for (i = 0; i < sizeof(icmp_hdr) + 64; i++)
		Transport_Network_Pool[Transport_Network_ReadIndex][i] = *((char*)icmp_header + i);
	tcp_size_of_package[Transport_Network_ReadIndex] = sizeof(icmp_hdr) + 64;
	for (i = 0; i < 4; i++)
		dest_ip[Transport_Network_ReadIndex][i] = source_ip[i];
	ip_type[Transport_Network_ReadIndex] = IP_ICMP;
	V(&Transport_Network_Mutex);
	V(&Transport_Network_Full);
	if (THREAD_DEBUG)
		printf("\n[接收][网络层][receive_from_datalink线程] 成功发送ICMP报文\n");
	return 0;
}
