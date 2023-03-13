#include "netstack.h"
#include "transport_tcp_socket.h"

const int mss = 5000;
const int buflen = sizeof(pse_header) + sizeof(tcp_header) + mss;
u_int8_t buf[buflen]={ 0 };

Socket* tcp_socket() {
	Socket* sockid = (Socket*)malloc(sizeof(Socket));	//��̬����TCPͨ����Ԫ��
	int i;
	srand((unsigned)time(NULL));
	for (i = 0; i < 4; i++)
		sockid->local_ip[i] = Local_IP[i];
	switch (host) {
		case CLIENT:
			sockid->local_port = 49152 + rand()%16384;
			break;
		case SERVER:
			sockid->local_port = -1;
			break;
		default:
			printf("Wrong Host Format!\n");
			return NULL;
			break;
	}
	for (i = 0; i < 4; i++)
		sockid->target_ip[i] = 0;
	sockid->target_port = -1;
	sockid->sock_type = SOCK_STREAM;
	return sockid;
}

int tcp_bind(Socket* sockid, u_int8_t* server_ip, u_int16_t server_port) {
	int i;
	//�󶨷�������ַ�Ͷ˿�
	if (host != SERVER)
		return 0;
	for (i = 0; i < 4; i++)
		sockid->local_ip[i] = server_ip[i];
	sockid->local_port = server_port;
	return 1;
}

int tcp_connect(Socket* sockid, u_int8_t* server_ip, u_int16_t server_port) {
	int i;
	if (host != CLIENT)
		return 0;
	tcp_terminate = 0;
	for (i = 0; i < 4; i++)
		sockid->target_ip[i] = server_ip[i];
	sockid->target_port = server_port;
	srand((unsigned)time(NULL));
	tcb = (tcp_control_block*)malloc(sizeof(tcp_control_block));
	tcb->client_initial_sequence = 10000 * (rand()%50);
	tcb->client_mss = mss;
	tcb->recv_window_size = 1;
	tcb->recv_cache_size = NUM_QUE;
	tcb->send_window_size = 1;
	tcb->send_cache_size = NUM_QUE;
	tcp_header* tcp_hdr = (tcp_header*)(buf + sizeof(pse_header));
	pse_header* pse_hdr = (pse_header*)buf;
	//��һ������
	for(i = 0; i < 4; i++)
		pse_hdr->src_ip[i] = sockid->local_ip[i];
	for (i = 0; i < 4; i++)
		pse_hdr->dest_ip[i] = sockid->target_ip[i];
	pse_hdr->reserve = 0;
	pse_hdr->protocol = IP_TCP;
	pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
	tcp_hdr->src_port = sockid->local_port;
	tcp_hdr->dest_port = sockid->target_port;
	tcp_hdr->sequence = tcb->client_initial_sequence;
	tcp_hdr->acknowledge = 0;
	tcp_hdr->header_length = (sizeof(tcp_header) / 4) << 4;
	tcp_hdr->flags = SYN;
	tcp_hdr->winsize = tcb->recv_window_size;
	tcp_hdr->checksum = 0;
	tcp_hdr->urgent_offset = 0;
	tcp_hdr->optional[0] = 2;	//��дMSSѡ���ֶ�
	tcp_hdr->optional[1] = 4;
	tcp_hdr->optional[2] = tcb->client_mss / 256;
	tcp_hdr->optional[3] = tcb->client_mss % 256;
	for (i = 0; i < tcb->client_mss; i++)
		buf[i + sizeof(pse_header) + sizeof(tcp_header)] = rand_data[i % 64];
	tcp_hdr->checksum = calculate_check_sum((ip_header*)buf, buflen);
	P(&Transport_Network_Empty);
	P(&Transport_Network_Mutex);
	for (i = sizeof(pse_header); i < buflen; i++)
		Transport_Network_Pool[Transport_Network_ReadIndex][i - sizeof(pse_header)] = buf[i];
	tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
	for (i = 0; i < 4; i++)
		dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
	ip_type[Transport_Network_ReadIndex] = IP_TCP;
	if (TCP_DEBUG)
		printf("\n[�ͻ���][�����][TCP]�ɹ����͵�һ������,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	Transport_Network_ReadIndex++;
	V(&Transport_Network_Mutex);
	V(&Transport_Network_Full);
	//�ڶ�������
	tcp_hdr->flags = 0;
	while ((tcp_hdr->flags & (ACK + SYN)) != (ACK + SYN)) {
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
		pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
		u_int16_t checksum = calculate_check_sum((ip_header*)buf, buflen);
		if (checksum != 0x0000 && checksum != 0xffff)
			continue;
		if (tcp_hdr->optional[0] != 2 || tcp_hdr->optional[1] != 4)	//δЯ��MSSѡ���ֶ�
			continue;
	}
	if (TCP_DEBUG)
		printf("\n[�ͻ���][�����][TCP]�ɹ����յڶ�������,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	tcb->server_initial_sequence = tcp_hdr->sequence;
	tcb->server_mss = tcp_hdr->optional[2] * 256 + tcp_hdr->optional[3];
	//����������
	for (i = 0; i < 4; i++)
		pse_hdr->src_ip[i] = sockid->local_ip[i];
	for (i = 0; i < 4; i++)
		pse_hdr->dest_ip[i] = sockid->target_ip[i];
	pse_hdr->reserve = 0;
	pse_hdr->protocol = IP_TCP;
	pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
	tcp_hdr->src_port = sockid->local_port;
	tcp_hdr->dest_port = sockid->target_port;
	tcp_hdr->sequence = tcb->client_initial_sequence + tcb->client_mss;
	tcp_hdr->acknowledge = tcb->server_initial_sequence + tcb->server_mss;
	tcp_hdr->header_length = (sizeof(tcp_header) / 4) << 4;
	tcp_hdr->flags = ACK;
	tcp_hdr->winsize = tcb->recv_window_size;
	tcp_hdr->checksum = 0;
	tcp_hdr->urgent_offset = 0;
	for (i = 0; i < 40; i++)
		tcp_hdr->optional[i] = 0;
	for (i = 0; i < tcb->client_mss; i++)
		buf[i + sizeof(pse_header) + sizeof(tcp_header)] = rand_data[i % 64];
	tcp_hdr->checksum = calculate_check_sum((ip_header*)buf, buflen);
	P(&Transport_Network_Empty);
	P(&Transport_Network_Mutex);
	for (i = sizeof(pse_header); i < buflen; i++)
		Transport_Network_Pool[Transport_Network_ReadIndex][i - sizeof(pse_header)] = buf[i];
	tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
	for (i = 0; i < 4; i++)
		dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
	ip_type[Transport_Network_ReadIndex] = IP_TCP;
	if (TCP_DEBUG)
		printf("\n[�ͻ���][�����][TCP]�ɹ����͵���������,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	Transport_Network_ReadIndex++;
	V(&Transport_Network_Mutex);
	V(&Transport_Network_Full);
	tcb->communication_mss = mss;
	seq_num = tcb->client_initial_sequence + tcb->client_mss;
	ack_num = tcb->server_initial_sequence + tcb->server_mss;
	return 1;
}

int tcp_listen(Socket* sockid, u_int8_t* client_ip, u_int16_t* client_port) {
	int i;
	if (host != SERVER)
		return 0;
	tcp_terminate = 0;
	srand((unsigned)time(NULL));
	tcb = (tcp_control_block*)malloc(sizeof(tcp_control_block));
	tcb->server_initial_sequence = 10000 * (rand() % 50);
	tcb->server_mss = mss;
	tcb->recv_window_size = 1;
	tcb->recv_cache_size = NUM_QUE;
	tcb->send_window_size = 1;
	tcb->send_cache_size = NUM_QUE;
	tcp_header* tcp_hdr = (tcp_header*)(buf + sizeof(pse_header));
	pse_header* pse_hdr = (pse_header*)buf;
	//��һ������
	tcp_hdr->flags = 0;
	while ((tcp_hdr->flags & SYN) != SYN) {
		P(&TCP_Recv_Full);
		P(&TCP_Recv_Mutex); 
		for (i = 0; i < TCP_Recv_Size[TCP_Recv_WriteIndex]; i++)
			*((u_int8_t*)tcp_hdr + i) = TCP_Recv_Pool[TCP_Recv_WriteIndex][i];
		for (i = 0; i < 4; i++)
			pse_hdr->src_ip[i] = TCP_Recv_Src_IP[TCP_Recv_WriteIndex][i];
		pse_hdr->length = TCP_Recv_Size[TCP_Recv_WriteIndex];
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
		u_int16_t checksum = calculate_check_sum((ip_header*)buf, buflen);
		if (checksum != 0x0000 && checksum != 0xffff)
			continue;
		if (tcp_hdr->optional[0] != 2 || tcp_hdr->optional[1] != 4)	//δЯ��MSSѡ���ֶ�
			continue;
	}
	if (TCP_DEBUG)
		printf("\n[������][�����][TCP]�ɹ����յ�һ������,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	tcb->client_initial_sequence = tcp_hdr->sequence;
	tcb->client_mss = tcp_hdr->optional[2] * 256 + tcp_hdr->optional[3];
	for (i = 0; i < 4; i++)
		client_ip[i] = pse_hdr->src_ip[i];
	*client_port = tcp_hdr->src_port;

	return 1;
}

int tcp_accept(Socket* sockid, u_int8_t* client_ip, u_int16_t client_port) {
	int i; 
	if (host != SERVER)
		return 0;
	tcp_header* tcp_hdr = (tcp_header*)(buf + sizeof(pse_header));
	pse_header* pse_hdr = (pse_header*)buf;
	for (i = 0; i < 4; i++)
		sockid->target_ip[i] = client_ip[i];
	sockid->target_port = client_port;
	//�ڶ�������
	for (i = 0; i < 4; i++)
		pse_hdr->src_ip[i] = sockid->local_ip[i];
	for (i = 0; i < 4; i++)
		pse_hdr->dest_ip[i] = sockid->target_ip[i];
	pse_hdr->reserve = 0;
	pse_hdr->protocol = IP_TCP;
	pse_hdr->length = sizeof(tcp_header) + tcb->server_mss;
	tcp_hdr->src_port = sockid->local_port;
	tcp_hdr->dest_port = sockid->target_port;
	tcp_hdr->sequence = tcb->server_initial_sequence;
	tcp_hdr->acknowledge = tcb->client_initial_sequence + tcb->client_mss;
	tcp_hdr->header_length = (sizeof(tcp_header) / 4) << 4;
	tcp_hdr->flags = ACK + SYN;
	tcp_hdr->winsize = tcb->recv_window_size;
	tcp_hdr->checksum = 0;
	tcp_hdr->urgent_offset = 0;
	tcp_hdr->optional[0] = 2;	//��дMSSѡ���ֶ�
	tcp_hdr->optional[1] = 4;
	tcp_hdr->optional[2] = tcb->server_mss / 256;
	tcp_hdr->optional[3] = tcb->server_mss % 256;
	for (i = 0; i < tcb->server_mss; i++)
		buf[i + sizeof(pse_header) + sizeof(tcp_header)] = rand_data[i % 64];
	tcp_hdr->checksum = calculate_check_sum((ip_header*)buf, buflen);
	P(&Transport_Network_Empty);
	P(&Transport_Network_Mutex);
	for (i = sizeof(pse_header); i < buflen; i++)
		Transport_Network_Pool[Transport_Network_ReadIndex][i - sizeof(pse_header)] = buf[i];
	tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
	for (i = 0; i < 4; i++)
		dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
	ip_type[Transport_Network_ReadIndex] = IP_TCP;
	if (TCP_DEBUG)
		printf("\n[������][�����][TCP]�ɹ����͵ڶ�������,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	Transport_Network_ReadIndex++;
	V(&Transport_Network_Mutex);
	V(&Transport_Network_Full);
	//����������
	tcp_hdr->flags = 0;
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
		pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
		u_int16_t checksum = calculate_check_sum((ip_header*)buf, buflen);
		if (checksum != 0x0000 && checksum != 0xffff)
			continue;
		if (tcp_hdr->sequence != tcb->client_initial_sequence + tcb->client_mss)	//���ڽ��մ�����
			continue;
	}
	if (TCP_DEBUG)
		printf("\n[������][�����][TCP]�ɹ����յ���������,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	tcb->communication_mss = mss;
	seq_num = tcb->server_initial_sequence + tcb->server_mss;
	ack_num = tcb->client_initial_sequence + tcb->client_mss;
	return 1;
}

int tcp_close(Socket* sockid) {
	int i;
	tcp_header* tcp_hdr = (tcp_header*)(buf + sizeof(pse_header));
	pse_header* pse_hdr = (pse_header*)buf;
	if (tcp_terminate) {		//�����ر�����
		//�����λ���
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
		tcp_hdr->flags = FIN;
		tcp_hdr->winsize = tcb->recv_window_size;
		tcp_hdr->checksum = 0;
		tcp_hdr->urgent_offset = 0;
		for (i = 0; i < tcb->client_mss; i++)
			buf[i + sizeof(pse_header) + sizeof(tcp_header)] = rand_data[i % 64];
		tcp_hdr->checksum = calculate_check_sum((ip_header*)buf, buflen);
		P(&Transport_Network_Empty);
		P(&Transport_Network_Mutex);
		for (i = sizeof(pse_header); i < buflen; i++)
			Transport_Network_Pool[Transport_Network_ReadIndex][i - sizeof(pse_header)] = buf[i];
		tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
		for (i = 0; i < 4; i++)
			dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
		ip_type[Transport_Network_ReadIndex] = IP_TCP;
		if (TCP_DEBUG)
			printf("\n[������][�����][TCP]�ɹ����͵����λ���,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
		Transport_Network_ReadIndex++;
		V(&Transport_Network_Mutex);
		V(&Transport_Network_Full);
		//���Ĵλ���
		tcp_hdr->flags = 0;
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
			pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
			u_int16_t checksum = calculate_check_sum((ip_header*)buf, buflen);
			if (checksum != 0x0000 && checksum != 0xffff)
				continue;
			if (tcp_hdr->sequence != ack_num) {
				printf("[TCP]Wrong sequence!\n");
				continue;
			}
		}
		seq_num = tcp_hdr->acknowledge;
		if (TCP_DEBUG)
			printf("\n[������][�����][TCP]�ɹ����յ��Ĵλ���,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
	}
	else {						//�����ر�����
		//��һ�λ���
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
		tcp_hdr->flags = FIN;
		tcp_hdr->winsize = tcb->recv_window_size;
		tcp_hdr->checksum = 0;
		tcp_hdr->urgent_offset = 0;
		for (i = 0; i < tcb->client_mss; i++)
			buf[i + sizeof(pse_header) + sizeof(tcp_header)] = rand_data[i % 64];
		tcp_hdr->checksum = calculate_check_sum((ip_header*)buf, buflen);
		P(&Transport_Network_Empty);
		P(&Transport_Network_Mutex);
		for (i = sizeof(pse_header); i < buflen; i++)
			Transport_Network_Pool[Transport_Network_ReadIndex][i - sizeof(pse_header)] = buf[i];
		tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
		for (i = 0; i < 4; i++)
			dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
		ip_type[Transport_Network_ReadIndex] = IP_TCP;
		if (TCP_DEBUG)
			printf("\n[�ͻ���][�����][TCP]�ɹ����͵�һ�λ���,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
		Transport_Network_ReadIndex++;
		V(&Transport_Network_Mutex);
		V(&Transport_Network_Full);
		//�ڶ��λ���
		tcp_hdr->flags = 0;
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
			pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
			u_int16_t checksum = calculate_check_sum((ip_header*)buf, buflen);
			if (checksum != 0x0000 && checksum != 0xffff)
				continue;
			if (tcp_hdr->sequence != ack_num) {
				printf("[TCP]Wrong sequence!\n");
				continue;
			}
		}
		seq_num = tcp_hdr->acknowledge;
		if (TCP_DEBUG)
			printf("\n[�ͻ���][�����][TCP]�ɹ����յڶ��λ���,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
		//�����λ���
		tcp_hdr->flags = 0;
		while ((tcp_hdr->flags & FIN) != FIN) {
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
			pse_hdr->length = sizeof(tcp_header) + tcb->client_mss;
			u_int16_t checksum = calculate_check_sum((ip_header*)buf, buflen);
			if (checksum != 0x0000 && checksum != 0xffff)
				continue;
			if (tcp_hdr->sequence != ack_num) {
				printf("[TCP]Wrong sequence!\n");
				continue;
			}
		}
		ack_num = tcp_hdr->sequence + tcb->communication_mss;
		if (TCP_DEBUG)
			printf("\n[�ͻ���][�����][TCP]�ɹ����յ����λ���,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
		//���Ĵλ���
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
		for (i = 0; i < tcb->client_mss; i++)
			buf[i + sizeof(pse_header) + sizeof(tcp_header)] = rand_data[i % 64];
		tcp_hdr->checksum = calculate_check_sum((ip_header*)buf, buflen);
		P(&Transport_Network_Empty);
		P(&Transport_Network_Mutex);
		for (i = sizeof(pse_header); i < buflen; i++)
			Transport_Network_Pool[Transport_Network_ReadIndex][i - sizeof(pse_header)] = buf[i];
		tcp_size_of_package[Transport_Network_ReadIndex] = pse_hdr->length;
		for (i = 0; i < 4; i++)
			dest_ip[Transport_Network_ReadIndex][i] = sockid->target_ip[i];
		ip_type[Transport_Network_ReadIndex] = IP_TCP;
		if (TCP_DEBUG)
			printf("\n[�ͻ���][�����][TCP]�ɹ����͵��Ĵλ���,seq=%d,ack=%d\n", tcp_hdr->sequence, tcp_hdr->acknowledge);
		Transport_Network_ReadIndex++;
		V(&Transport_Network_Mutex);
		V(&Transport_Network_Full);
	}
	free(tcb);
	for (i = 0; i < 4; i++)
		sockid->target_ip[i] = 0;
	sockid->target_port = -1;
	return 1;
}

