#ifndef NET_CONTROL_CLINET_H_
#define NET_CONTROL_CLINET_H_

#include <arpa/inet.h>

#define TCP_BUFFER_SIZE	1024//max buff of receive buffer for tcp

typedef struct{//tcp client class
		unsigned short  	server_port;
		char 				server_ip[64];
		int 				socket_fd;//socket
		struct 				sockaddr_in server_socket_addr;
}ts_tcp_client;



#endif /* NET_CONTROL_CLINET_H_ */
