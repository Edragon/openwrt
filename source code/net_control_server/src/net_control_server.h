#ifndef NET_CONTROL_SERVER_H
#define NET_CONTROL_SERVER_H

#include <netinet/in.h>

#define MAX_CLIENT_NUM		64//max client connected to server
#define MAX_QUE_CONN_NUM	20//max number of client waiting for connection
#define TCP_BUFFER_SIZE	1024//max buff of receive buffer for tcp

#define KEEP_ALIVE_ENABLE	1 // 开启keepalive属性
#define KEEP_IDLE			30// 如该连接在xx秒内没有任何数据往来,则进行探测
#define KEEP_INTERVAL		5// 探测时发包的时间间隔为xx秒
#define KEEP_COUNT			3// 探测尝试的次数.

typedef struct{
	int client_fd;//tcp client fd
	struct sockaddr_in listendAddr;//tcp client addr and port
}ts_client_info;

typedef struct{//tcp_server class
	ts_client_info s_client_info[MAX_CLIENT_NUM];
	int server_port;
}ts_tcp_server;

#endif/*NET_CONTROL_SERVER_H*/
