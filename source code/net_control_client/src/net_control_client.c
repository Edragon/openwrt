/*
 * net_control_clinet.c
 *
 * Copyright (C) 2017 wurobinson <wurobinson@zhuotk.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is written for JS7628 and JS9331 development board ,
 * for socket test
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "net_control_client.h"

ts_tcp_client s_tcp_client;

static int connect_to_server(void)
{
	while(connect(s_tcp_client.socket_fd, (struct sockaddr *) &(s_tcp_client.server_socket_addr),
			sizeof(struct sockaddr)) != 0){
		//if connect error reconnect after 5 seconds
		perror("connect");
		printf("***reconnect after 5s***\n");
		sleep(5);
		printf("***reconnect...***\n");
	}
	printf("***connected***\n");
#if 1//test message send
	char test_msg[] = "This is from client";
	send(s_tcp_client.socket_fd, test_msg ,sizeof(test_msg),0);
#endif
	return 0;
}

/*
 * 接收socket数据函数.
 * client_fd - 客户端连接的socket。
 */
static int receive_packet(int client_fd)
{
	unsigned char 	buf[TCP_BUFFER_SIZE];
	int		recvbytes;
	while(1){
		/*接收*/
		bzero(buf,sizeof(buf));

		recvbytes = recv(client_fd,buf,TCP_BUFFER_SIZE,0);
		printf("Receive %d bytes\n",recvbytes);

		if (recvbytes <= 0){//receive error or disconnected
			perror("recv");
			/*reconnect to server*/
//			printf("close socket id = %d\n", s_tcp_client.socket_fd);
			close(s_tcp_client.socket_fd);
			s_tcp_client.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (s_tcp_client.socket_fd == -1){
				perror("socket");
				return -1;
			}
//			printf("new socket id = %d\n", s_tcp_client.socket_fd);
			if (connect_to_server() != 0){
				printf("###connect_to_server error###\n");
				return -1;
			}
		}else{//receive success
#if 1//test
			int i;
			printf("***GET:\n");
			for (i = 0; i < recvbytes; i++){
				printf("0x%02X  %c\n", *(buf+i), *(buf+i));
			}
#endif
		}
	}
	return 0;
}

/*tcp clinet running function*/
int tcp_client_start(void)
{

	printf("***connect to %s.....***\n", s_tcp_client.server_ip);
	/*connect to server*/
	if (connect_to_server() != 0){
		printf("###connect_to_server error###\n");
		return -1;
	}
	receive_packet(s_tcp_client.socket_fd);
	return 0;
}

/*
 * tcp_client initialize function
 * port_num -  TCP server port number
 * server_ip - TCP server ip
 * */
int tcp_client_init(unsigned short port_num, char *server_ip)
{
	int res;
	struct in_addr test_addr;
	/*initialize variable*/
	if (port_num > 0){
		s_tcp_client.server_port = port_num;
	}else{
		printf("###invalid tcp server port:%d###\n", port_num);
		return -1;
	}

	if (server_ip == NULL){
		printf("###server_ip cannot be NULL###\n");
		return -1;
	}else{
		strcpy(s_tcp_client.server_ip, server_ip);//record ip
	}

	/*建立socket描述符*/
	if ((s_tcp_client.socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		return -1;
	}
	printf("socket id = %d\n", s_tcp_client.socket_fd);

	/*
	 * 填充服务器sockaddr结构
	 */
	bzero(&(s_tcp_client.server_socket_addr), sizeof(struct sockaddr_in));
	s_tcp_client.server_socket_addr.sin_family		 	= AF_INET;
	inet_pton(AF_INET, s_tcp_client.server_ip, &(s_tcp_client.server_socket_addr.sin_addr.s_addr));
	s_tcp_client.server_socket_addr.sin_port				= htons(s_tcp_client.server_port);
	bzero(&(s_tcp_client.server_socket_addr.sin_zero), 8);

	return 0;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int res;
	pthread_t 	tcp_thread_id;//返回的线程值
	char tcp_server_port[256];
	char tcp_server_ip[256];

	if (argc >= 2){
		if (strcmp(argv[1],"-v") == 0){
			printf("net_control_client v1.0\n");
			return 0;
		}else if (strcmp(argv[1],"-h") == 0){
			printf("-v for version\n");
			printf("-h for help\n");
			printf("tcp_connect <IP> <port>\n");
			return 0;
		}else if (strcmp(argv[1], "tcp_connect") == 0){
			/*tcp demo*/
			strcpy(tcp_server_ip, argv[2]);
			strcpy(tcp_server_port, argv[3]);
			/*initialize functions*/

			res = tcp_client_init((unsigned short)atoi(tcp_server_port), tcp_server_ip);//set tcp clinet setting
			if (res == -1){
				printf("###tcp_server_init error###\n");
				return -1;
			}

			//create thread for TCP communication
			pthread_create(&tcp_thread_id, NULL, (void *)tcp_client_start, NULL);
		}else{
			printf("Unknown argument %s\n",argv[1]);
			return -1;
		}
	}
	while(1){
		if (i < 100){
			i++;
		}else{
			i = 0;
		}
//		printf("hello_world:%d\n", i);
		sleep(1);
	}
	exit(0);
}
