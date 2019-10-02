/*
 * net_control_server.c
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "net_control_server.h"

ts_tcp_server s_tcp_server;

/*
 * 处理客户端的连接的线程函数.该线程调用解包函数解包，最后
 * 调用解析函数进行解析.
 * client_fd - 客户端连接的socket。
 */
static int thrd_receive_packet(void *client_fd)
{
	unsigned char 	buf[TCP_BUFFER_SIZE];
	int				recvbytes;
	int				fd;
	int 			keepalive 	= KEEP_ALIVE_ENABLE;
	int 			keepidle 	= KEEP_IDLE;
	int 			keepinterval = KEEP_INTERVAL;
	int 			keepcount 	= KEEP_COUNT;

	fd		= *((int *)(client_fd));
	printf("new client_fd = %d\n",fd);
	printf("New thread %d\n",(int)pthread_self());

	while(1){
		/*接收*/
		bzero(buf,sizeof(buf));
		if ((recvbytes = recv(fd,buf,TCP_BUFFER_SIZE,0)) <= 0){
			/*receive error or disconnected */
			printf("Receive %d bytes\n",recvbytes);
			perror("recv");
			printf("close client_fd %d\n", fd);
			close(fd);
			pthread_exit(NULL);
			return -1;
		}

	#if 0//test
		send(fd,buf,recvbytes,0);
	#endif
	#if 1//test
		int i;
		printf("***GET:\n");
		for (i = 0; i < recvbytes; i++){
			printf("0x%02X %c\n", *(buf+i), *(buf+i));
		}
		printf("from fd:%d***\n",fd);
	#endif
	}
	//	close(fd);
	return 0;

}

/*tcp server running function*/
int tcp_server_start(void)
{
	int 		socket_fd;//服务端socket
	struct 		sockaddr_in server_socket_addr;
	struct 		sockaddr_in client_socket_addr;
	int			i;
	int			res;

	/*建立socket描述符*/
	if ((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("socket");
		return -1;
	}
//	printf("socket id = %d\n",socket_fd);

	/*
	 * 服务器填充sockaddr结构
	 */
	bzero(&server_socket_addr,sizeof(struct sockaddr_in));
	server_socket_addr.sin_family		 	= AF_INET;
	server_socket_addr.sin_addr.s_addr 		= htonl(INADDR_ANY);
	server_socket_addr.sin_port				= htons(s_tcp_server.server_port);

	/* 绑定套接字*/
	i = 1;//允许重复使用本地地址与套接字进行绑定
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
	if (bind(socket_fd,(struct sockaddr*)&server_socket_addr,
			sizeof(struct sockaddr)) == -1){
		perror("bind");
		return -1;
	}
//	printf("binding suceess!\n");

	/** 监听，创建未处理请求的队列*/
	if (listen(socket_fd, MAX_QUE_CONN_NUM) == -1){
		perror("listen");
		return -1;
	}
	printf("listening...\n");

//	printf("listen address = 0x%X:%d\n", listendAddr.sin_addr, ntohs(listendAddr.sin_port));
	while (1){
		/*
		 * 等待客户端连接,每获得连接后则创建新线程
		 */
		int 		client_fd;//临时客户端描述符
		pthread_t 	thread;//返回的线程值
		socklen_t sin_size= sizeof(struct sockaddr);
		if ((client_fd = accept(socket_fd,
				(struct sockaddr *)&client_socket_addr,(socklen_t *)&sin_size)) == -1){
			perror("accept");
			return -1;
		}

//		printf("client address = %s:%d\n", inet_ntoa(client_socket_addr.sin_addr), ntohs(client_socket_addr.sin_port));

		if (pthread_create(&thread, NULL, (void *)thrd_receive_packet,
				(void *)(&client_fd)) != 0){
			perror("pthread_create");
			return  -1;
		}
	}
	close(socket_fd);
	return 0;
}

/*
 * tcp_server initialize function
 * port_num - port number for TCP server
 * */
int tcp_server_init(int server_port)
{
	int i;
	/*initialize variable*/
	if (server_port > 0){
		s_tcp_server.server_port = server_port;
	}else{
		printf("###invalid tcp server port:%d###\n", server_port);
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int res;
	pthread_t 	tcp_thread_id;//返回的线程值
	printf("***net_control_server***\n");
	if (argc >= 2){
		if (strcmp(argv[1],"-v") == 0){
			printf("net_control_server v1.0\n");
			return 0;
		}else if (strcmp(argv[1],"-h") == 0){
			printf("-v for version\n");
			printf("-h for help\n");
			printf("tcp_server <port>\n");
			return 0;
		}else if(strcmp(argv[1], "tcp_server") == 0){
			/*start tcp server*/
			res = tcp_server_init((unsigned int)atoi(argv[2]));//set tcp server setting
			if (res == -1){
				printf("###tcp_server_init error###\n");
				return -1;
			}
			//create thread for TCP communication
			pthread_create(&tcp_thread_id, NULL, (void *)tcp_server_start, NULL);
		}else{
			printf("Unknown argument %s\n",argv[1]);
			return -1;
		}
	}
	while(1){
#if 0
		if (i < 100){
			i++;
		}else{
			i = 0;
		}
		printf("hello_world:%d\n", i);
#endif
		sleep(1);
	}

	exit(0);
}
