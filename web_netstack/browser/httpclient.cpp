/*************************************************************************
 * 
 * Copyright (c) 2016-2017 by xuwm All Rights Reserved
 *
 * FILENAME:  WebClient.c
 *
 * PURPOSE :  HTTP Client: Get 
 *  
 * AUTHOR  :  yao ye(NPU)
 * 
 **************************************************************************/
//#include "stdafx.h"
#include <stdio.h>
#include<stdlib.h>
#include <WinSock2.h>
#include <pcap.h>
#define HAVE_REMOTE 
#pragma comment(lib, "ws2_32.lib")          /* WinSockʹ�õĿ⺯�� */
#pragma comment(lib,"Packet.lib")
#pragma warning(disable:4996)
//#include "browser.h"

/* ���峣�� */
#define HTTP_DEF_PORT     80  /* ���ӵ�ȱʡ�˿� */
#define HTTP_BUF_SIZE       1024  /* �������Ĵ�С   */
#define HTTP_HOST_LEN    256  /* ���������� */
 
const char *http_req_hdr_tmpl = 
    "GET %s HTTP/1.1\r\n"
    "Accept: image/gif, image/jpeg, */*\r\n"
    "Accept-Language: zh-cn\r\n"
    //"Accept-Encoding: gzip, deflate\r\n"
    "Host: %s:%d\r\n"
    "User-Agent: WangHanmo's Browser <0.1>\r\n"
    "Connection: Keep-Alive\r\n\r\n";
 
 
/**************************************************************************
 *
 * ��������: ���������в���, �ֱ�õ�������, �˿ںź��ļ���. �����и�ʽ:
 *           [http://www.baidu.com:8080/index.html]
 *
 * ����˵��: [IN]  buf, �ַ���ָ������;
 *           [OUT] host, ��������;
 *           [OUT] port, �˿�;
 *           [OUT] file_name, �ļ���;
 *
 * �� �� ֵ: void.
 *
 **************************************************************************/
int http_parse_request_url(const char* buf, char* host,
    unsigned short* port, char* file_name, unsigned long* addr)
{
    int length = 0;
    char port_buf[8];
    char* buf_end = (char*)(buf + strlen(buf));
    char* begin, * host_end, * colon, * file;

    /* ���������Ŀ�ʼλ�� */

    begin = const_cast<char*>(strstr(buf, "//"));
    begin = (begin ? begin + 2 : const_cast<char*>(buf));

    colon = strchr(begin, ':');
    host_end = strchr(begin, '/');

    if (host_end == NULL)
    {
        host_end = buf_end;
    }
    else
    {   /* �õ��ļ��� */
        file = host_end - 1;
        if (file && (file + 1) != buf_end)
            strcpy(file_name, file + 1);
    }
    if (colon) /* �õ��˿ں� */
    {
        colon++;

        length = host_end - colon;
        memcpy(port_buf, colon, length);
        port_buf[length] = 0;
        *port = atoi(port_buf);

        host_end = colon - 1;
    }
    host_end--;
    /* �õ�������Ϣ */
    length = host_end - begin;
    memcpy(host, begin, length);
    host[length] = 0;
    /* �õ�IP��ַ */
    if (strcmp(host, "www.wanghanmo.com") == 0)
        *addr = 0x84f6a8c0;
    else
        *addr = 0x00000000;
    return 1;
}
 //http://www.wanghanmo.com[:8080]/index.html
 //http://www.wanghanmo.com[:8080]/cgi-bin/adder?1500&212
 
int httpclient(char* url)
{
    WSADATA wsa_data;
    SOCKET  http_sock = 0;         /* socket ��� */
    struct sockaddr_in serv_addr;  /* ��������ַ */
    struct hostent *host_ent;
     
    int result = 0, send_len;
    char data_buf[HTTP_BUF_SIZE];
    char host[HTTP_HOST_LEN] = "127.0.0.1";
    unsigned short port = HTTP_DEF_PORT;
    unsigned long addr;
    char file_name[HTTP_HOST_LEN] = "index.html";
    char file_name_forsave[HTTP_HOST_LEN] = "index.html";
    FILE *file_web;
    char http_buf[HTTP_BUF_SIZE];
    int http_len;
    
    http_parse_request_url(url, host, &port, file_name, &addr); 

    printf("http://%s[:%d]%s\n", host, port, file_name);
    WSAStartup(MAKEWORD(2, 0), &wsa_data); /* ��ʼ�� WinSock ��Դ */
    
    printf("addr:%x\n", addr);
    /* ��������ַ */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = addr;
 
    http_sock = socket(AF_INET, SOCK_STREAM, 0); /* ���� socket */
    result = connect(http_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (result == SOCKET_ERROR) /* ����ʧ�� */
    {
        closesocket(http_sock);
        printf("[Web] fail to connect, error = %d\n", WSAGetLastError());
        return -1; 
    }
    printf("[Web] succeed to connect\n");
    /* ���� HTTP ���� */
    send_len = sprintf(data_buf, http_req_hdr_tmpl, file_name, host, port);
    printf("http request:\n%s\n", data_buf);

    result = send(http_sock, data_buf, send_len, 0);
    if (result == SOCKET_ERROR) /* ����ʧ�� */
    {
        printf("[Web] fail to send, error = %d\n", WSAGetLastError());
        return -1; 
    }
    printf("[Web]succeed to send\n");
    file_web = fopen(file_name_forsave, "w+");
    memset(http_buf, 0, HTTP_BUF_SIZE);
    http_len = 0;
    do /* ������Ӧ�����浽�ļ��� */
    {
        memset(data_buf, 0, HTTP_BUF_SIZE);
        result = recv(http_sock, data_buf, HTTP_BUF_SIZE, 0);
        if (result > 0)
        {
            memcpy(http_buf + http_len, data_buf, result);
            http_len += result;
        }
    } while(result > 0);

    int i, length;
    char tmp[10];
    http_buf[http_len] = 0;
    char* begin = const_cast<char*>(strstr(http_buf, "Content-length: "));
    begin = begin ? begin + 16 : const_cast<char*>(http_buf);
    for (i = 0; i < 10; i++)
        if (begin[i] < '0' || begin[i] > '9')
            break;
    length = i;
    memcpy(tmp, begin, length);
    tmp[length] = 0;
    i = atoi(tmp);
    fwrite(http_buf + http_len - i, 1, i, file_web);
    /* ����Ļ����� */
    //printf("%s", data_buf);
    fclose(file_web);
    closesocket(http_sock);
    WSACleanup();
 
    return 0;
}
 
 //������vs2010�еģ����һ��VC�����г��򣬰�����ĳ���ֱ�ӷŵ��������Ӧ��cpp�ļ��У�Ȼ����뼴�ɡ�
