#include "netstack.h"
#include "transport_tcp_commun.h"
u_int8_t tcp_send_buffer[MAX_TCP_PACKAGE_SIZE + sizeof(pse_header)] = { 0 };
u_int8_t tcp_recv_buffer[MAX_TCP_PACKAGE_SIZE + sizeof(pse_header)] = { 0 };


int tcp_send(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t flags) {
	int i, cnt = 0;
	tcp_header* tcp_hdr = (tcp_header*)(tcp_send_buffer + sizeof(pse_header));
	pse_header* pse_hdr = (pse_header*)tcp_send_buffer;
	u_int8_t* buf_ptr = tcp_send_buffer + sizeof(pse_header) + sizeof(tcp_header);
	while (cnt < buflen) {
		//构造TCP报文
		for (i = 0; i < 4; i++)
			pse_hdr->src_ip[i] = sockid->local_ip[i];
		for (i = 0; i < 4; i++)
			pse_hdr->dest_ip[i] = sockid->target_ip[i];
		pse_hdr->reserve = 0;
		pse_hdr->protocol = IP_TCP;
		pse_hdr->length = sizeof(tcp_header) + tcb->communication_mss;
		tcp_hdr->src_port = sockid->local_port;
		tcp_hdr->dest_port = sockid->target_port;
		tcp_hdr->sequence = seq_num;
		tcp_hdr->acknowledge = ack_num;
		tcp_hdr->header_length = (sizeof(tcp_header) / 4) << 4;
		tcp_hdr->flags = 0;
		tcp_hdr->winsize = tcb->recv_window_size;
		tcp_hdr->checksum = 0;
		tcp_hdr->urgent_offset = 0;
		for (i = 0; i < tcb->communication_mss && cnt < buflen; i++, cnt++)
			buf_ptr[i] = buf[cnt];
		for (; i < tcb->communication_mss; i++)
			buf_ptr[i] = 0;
		tcp_hdr->checksum = calculate_check_sum((ip_header*)tcp_send_buffer, sizeof(pse_header) + pse_hdr->length);
		//发送TCP报文
		P(&Transport_Network_Empty);
		P(&Transport_Network_Mutex);
		for (i = 0; i < pse_hdr->length; i++)
			Transport_Network_Pool[Transport_Network_ReadIndex][i] = *((u_int8_t*)tcp_hdr + i);
		//for (i = 0; i < pse_hdr->length - sizeof(tcp_header); i++)
		//	printf("%c", buf_ptr[i]);
		tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
		for (i = 0; i < 4; i++)
			dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
		ip_type[Transport_Network_ReadIndex] = IP_TCP;
		if (TCP_DEBUG)
			printf("\n[发送][传输层][TCP]成功发送第%d个TCP报文,seq=%d,ack=%d\n", Transport_Network_ReadIndex, seq_num, ack_num);
		Transport_Network_ReadIndex++;
		V(&Transport_Network_Mutex);
		V(&Transport_Network_Full);
		//等待ACK应答
		while ((tcp_hdr->flags & ACK) != ACK) {
			P(&TCP_Recv_Full);
			P(&TCP_Recv_Mutex);
			for (i = 0; i < TCP_Recv_Size[TCP_Recv_WriteIndex]; i++)
				*((u_int8_t*)tcp_hdr + i) = TCP_Recv_Pool[TCP_Recv_WriteIndex][i];
			for (i = 0; i < 4; i++)
				pse_hdr->src_ip[i] = TCP_Recv_Src_IP[TCP_Recv_WriteIndex][i];
			TCP_Recv_WriteIndex++;
			V(&TCP_Recv_Mutex);
			V(&TCP_Recv_Empty);
			if (tcp_hdr->dest_port != sockid->local_port) {
				printf("[TCP]Wrong Destination Port! %d-%d\n", tcp_hdr->dest_port, sockid->local_port);
				continue;
			}
			for (i = 0; i < 4; i++)
				pse_hdr->dest_ip[i] = Local_IP[i];
			pse_hdr->reserve = 0;
			pse_hdr->protocol = IP_TCP;
			pse_hdr->length = sizeof(tcp_header) + tcb->communication_mss;
			u_int16_t checksum = calculate_check_sum((ip_header*)tcp_send_buffer, sizeof(pse_header) + pse_hdr->length);
			if (checksum != 0x0000 && checksum != 0xffff)
				continue;
			if (tcp_hdr->sequence != ack_num) {
				printf("[TCP]Wrong sequence!\n");
				continue;
			}
			seq_num = tcp_hdr->acknowledge;
			if (TCP_DEBUG)
				printf("\n[发送][传输层][TCP]成功接收第%d个ACK应答,seq=%d,ack=%d\n", Transport_Network_ReadIndex, tcp_hdr->sequence, tcp_hdr->acknowledge);
		}
	}
	return 1;
}

int tcp_recv(Socket* sockid, u_int8_t* buf, int buflen, u_int8_t flags) {
	int i, len;
	int TCP_Recv_WriteIndex_tmp;
	tcp_header* tcp_hdr = (tcp_header*)(tcp_recv_buffer + sizeof(pse_header));
	pse_header* pse_hdr = (pse_header*)tcp_recv_buffer;
	u_int8_t* buf_ptr = tcp_recv_buffer + sizeof(pse_header) + sizeof(tcp_header);
	//接收TCP报文
	while (1) {
		P(&TCP_Recv_Full);
		P(&TCP_Recv_Mutex);
		for (i = 0; i < TCP_Recv_Size[TCP_Recv_WriteIndex]; i++)
			*((u_int8_t*)tcp_hdr + i) = TCP_Recv_Pool[TCP_Recv_WriteIndex][i];
		for (i = 0; i < 4; i++)
			pse_hdr->src_ip[i] = TCP_Recv_Src_IP[TCP_Recv_WriteIndex][i];
		TCP_Recv_WriteIndex_tmp = TCP_Recv_WriteIndex;
		TCP_Recv_WriteIndex++;
		V(&TCP_Recv_Mutex);
		V(&TCP_Recv_Empty);
		if (tcp_hdr->dest_port != sockid->local_port) {
			printf("[TCP]Wrong Destination Port! %d-%d\n", tcp_hdr->dest_port, sockid->local_port);
			continue;
		}
		for (i = 0; i < 4; i++)
			pse_hdr->dest_ip[i] = Local_IP[i];
		pse_hdr->reserve = 0;
		pse_hdr->protocol = IP_TCP;
		pse_hdr->length = sizeof(tcp_header) + tcb->communication_mss;
		u_int16_t checksum = calculate_check_sum((ip_header*)tcp_recv_buffer, sizeof(pse_header) + pse_hdr->length);
		if (checksum != 0x0000 && checksum != 0xffff)
			continue;
		if (tcp_hdr->sequence != ack_num) {
			printf("[TCP]Wrong sequence!\n");
			continue;
		}
		ack_num = tcp_hdr->sequence + tcb->communication_mss;
		if ((tcp_hdr->flags & FIN) == FIN) {
			tcp_terminate = 1; 
			if (TCP_DEBUG)
				printf("\n[服务器][传输层][TCP]成功接收第一次挥手,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
			break;
		}
		if (TCP_DEBUG)
			printf("\n[接收][传输层][TCP]成功接收第%d个TCP报文,seq=%d,ack=%d\n", TCP_Recv_WriteIndex_tmp, tcp_hdr->sequence, tcp_hdr->acknowledge);
		break;
	}
	if (pse_hdr->length - sizeof(tcp_header) > buflen) {
		printf("[TCP]Buffer Overflow!\n");
		return -1;
	}
	for (i = 0; i < pse_hdr->length - sizeof(tcp_header); i++)
		buf[i] = buf_ptr[i];
	//if(tcp_terminate == 0)
	//	for (i = 0; i < pse_hdr->length - sizeof(tcp_header); i++)
	//		printf("%c",buf[i]);
	len = pse_hdr->length - sizeof(tcp_header);
	//构造ACK应答
	for (i = 0; i < 4; i++)
		pse_hdr->src_ip[i] = sockid->local_ip[i];
	for (i = 0; i < 4; i++)
		pse_hdr->dest_ip[i] = sockid->target_ip[i];
	pse_hdr->reserve = 0;
	pse_hdr->protocol = IP_TCP;
	pse_hdr->length = sizeof(tcp_header) + tcb->communication_mss;
	tcp_hdr->src_port = sockid->local_port;
	tcp_hdr->dest_port = sockid->target_port;
	tcp_hdr->sequence = seq_num;
	tcp_hdr->acknowledge = ack_num;
	tcp_hdr->header_length = (sizeof(tcp_header) / 4) << 4;
	tcp_hdr->flags = ACK;
	tcp_hdr->winsize = tcb->recv_window_size;
	tcp_hdr->checksum = 0;
	tcp_hdr->urgent_offset = 0;
	for (i = 0; i < tcb->communication_mss; i++)
		buf_ptr[i] = rand_data[i % 64];
	tcp_hdr->checksum = calculate_check_sum((ip_header*)tcp_recv_buffer, sizeof(pse_header) + pse_hdr->length);
	//发送ACK应答
	P(&Transport_Network_Empty);
	P(&Transport_Network_Mutex);
	for (i = 0; i < pse_hdr->length; i++)
		Transport_Network_Pool[Transport_Network_ReadIndex][i] = *((u_int8_t*)tcp_hdr + i);
	tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
	for (i = 0; i < 4; i++)
		dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
	ip_type[Transport_Network_ReadIndex] = IP_TCP;
	if (tcp_terminate) {
		if (TCP_DEBUG)
			printf("\n[服务器][传输层][TCP]成功发送第二次挥手,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	}
	else {
		if (TCP_DEBUG)
			printf("\n[接收][传输层][TCP]成功发送第%d个ACK应答,seq=%d,ack=%d\n", Transport_Network_ReadIndex, seq_num, ack_num);
	}
	Transport_Network_ReadIndex++;
	V(&Transport_Network_Mutex);
	V(&Transport_Network_Full);
	return len;
}

