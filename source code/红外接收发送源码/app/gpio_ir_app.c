/*
 * gpio_ir_app.c
 *
 *  Created on: 2015-5-10
 *      Author: robinson <wurobinson@joysince.com>
 * Description: This program is written for js9331 development board ,
 * 				 work with gpio_ir_driver
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/errno.h>


#include "gpio_ir_app.h"

ts_gpio_ir_app s_gpio_ir_app;

//For command options
static const char *optstring = "d:hv";
static const char *usage = "\
Usage: gpio_ir_app [option] [option parameter]\n\
-h          display help information\n\
-d  <path>  device path\n\
";
static const char *fifo_usage = "\
Usage: echo [option] [parameter] > /tmp/gpio_ir_fifo\n\
start_ir_receive\n\
stop_ir_receive\n\
send_ir_data      <0xdata>  send a 32 bits data\n\
help                        display help information\n\
exit                        exit this program\n\
";

//analyze input str for pwm
static int analyze(char *str)
{
	char 		cmd[MAX_BUF_SIZE];
	char 		para[MAX_BUF_SIZE];

	memset(cmd, 0, MAX_BUF_SIZE);
	memset(para, 0, MAX_BUF_SIZE);
	sscanf(str, "%s %s", cmd, para);
	printf("cmd:%s\n", cmd);
	printf("parameter:%s\n", para);

	if (strcmp(cmd, "start_ir_receive") == 0){
		printf("***start_receive***\n");
		ioctl(s_gpio_ir_app.dev_fd, GPIO_IR_START_RCV, 0);
	}else if(strcmp(cmd, "stop_ir_receive") == 0){
		printf("***stop_receive***\n");
		ioctl(s_gpio_ir_app.dev_fd, GPIO_IR_STOP_RCV, 0);
	}else if(strcmp(cmd, "send_ir_data") == 0){
		unsigned int ir_data;
		printf("***send_ir_data***\n");
		sscanf(para, "0x%x", &ir_data);
		write(s_gpio_ir_app.dev_fd, &ir_data, sizeof(unsigned int));
	}
	else if (strcmp(cmd, "exit") == 0){
		printf("***gpio_ir_app EXIT***\n");
		close(s_gpio_ir_app.fifo_fd);
		close(s_gpio_ir_app.dev_fd);
		exit(0);
	}else if (strcmp(cmd, "help") == 0){
		printf("%s",fifo_usage);
	}
	else{
		printf("##Unknown cmd:%s###\n",cmd);
		printf("***Try `help' for more information.***\n");
	}
	return -EINVAL;

	return 0;
}

/* -------------------------------------------------------------------
 * Parse settings from app's command line.
 * ------------------------------------------------------------------- */
int parse_opt(const char opt, const char *optarg)
{
	switch(opt){
	case 'h':
		printf("%s\n",usage);
		exit(0);
		break;
	case 'd':
		printf("option:d\n");
		strcpy(s_gpio_ir_app.dev_path, optarg);
		break;
	case 'v':
		printf("gpio_ir_app Version 1.0, for js9331 development board\n");
		exit(0);
		break;
	default:
		printf("###Unknown option:%c###\n",opt);
		exit(0);
		break;
	}
	return 0;
}



int main(int argc,char *argv[])
{
	char opt;
	//initiation
	s_gpio_ir_app.dev_fd 	= -1;
	s_gpio_ir_app.fifo_fd	= -1;
	strcpy(s_gpio_ir_app.dev_path, GPIO_IR_DEVICE_PATH);

	//parse
	while((opt = getopt(argc, argv, optstring)) != -1){
//		printf("opt:%c\n",opt);
//		printf("optarg:%s\n",optarg);
//		printf("optind:%d\n",optind);
//		printf("argv[optind-1]:%s\n",argv[optind -1]);
		parse_opt(opt, optarg);
	}

	//check fifo file
	if (access(GPIO_IR_FIFO_PATH,F_OK) == -1){
		printf("**%s not exit,now create it**\n",GPIO_IR_FIFO_PATH);
		if (mkfifo(GPIO_IR_FIFO_PATH,0666) < 0){
			printf("##Can't create fifo file!!##\n");
			return -1;
		}
	}
	//open fifo
	s_gpio_ir_app.fifo_fd = open(GPIO_IR_FIFO_PATH,O_RDONLY | O_NONBLOCK);
	if (s_gpio_ir_app.fifo_fd == -1){
		printf("###Open %s ERROR!###",GPIO_IR_FIFO_PATH);
		return -1;
	}

	//open device
	s_gpio_ir_app.dev_fd = open(s_gpio_ir_app.dev_path, O_RDWR);
	if (s_gpio_ir_app.dev_fd == -1){
		printf("###open %s ERROR###\n",s_gpio_ir_app.dev_path);
		perror("###open###");
		return -1;
	}
//	printf("***s_gpio_ir_app.dev_path:%s***\n",s_gpio_ir_app.dev_path);
//	printf("***s_gpio_ir_app.dev_fd:%d***\n",s_gpio_ir_app.dev_fd);

	//read fifo message

	char rec_buf[MAX_RCV_BUF_SIZE];
	int buf_count;
	ts_ir_rcv_code s_ir_rcv_code;

	while(1){
		buf_count	= 0;
		//read chars one by one in the fifo,until get '\n'
		while(read(s_gpio_ir_app.fifo_fd, rec_buf + buf_count, 1) > 0){
			if (rec_buf[buf_count] == '\n'){//detected a '\n'? we get a line
				analyze(rec_buf);
				break;
			}
//			printf("GET:%c\n",rec_buf[buf_count]);
			++buf_count;
		}

		while(read(s_gpio_ir_app.dev_fd, &s_ir_rcv_code, sizeof(ts_ir_rcv_code))){
			if (s_ir_rcv_code.code_type == NEC_CODE_TYPE_NOMAL){
				printf("***get nec code_data:0x%08X***\n", s_ir_rcv_code.code_data);
				printf("***address:0x%02X***\n", (s_ir_rcv_code.code_data >> 0) & 0xFF);
				printf("***not_address:0x%02X***\n", (s_ir_rcv_code.code_data >> 8) & 0xFF);
				printf("***command:0x%02X***\n", (s_ir_rcv_code.code_data >> 16) & 0xFF);
				printf("***not_command:0x%02X***\n",(s_ir_rcv_code.code_data >> 24) & 0xFF);
			}else if (s_ir_rcv_code.code_type == NEC_CODE_TYPE_REPEAT){
				printf("***get repeat code***\n");
			}else{
				printf("###Unknown command:0x%02X###\n", s_ir_rcv_code.code_type);
			}
		}

#if 0
		if (i < 300){
			i++;
		}else{
			i = 0;
			s_ir_rcv_code.code_data = 0xCA010001;
			s_ir_rcv_code.code_type = NEC_CODE_TYPE_REPEAT;
			write(s_gpio_ir_app.dev_fd, &s_ir_rcv_code, sizeof(ts_ir_rcv_code));
			printf("***write***\n");
		}
#endif
		usleep(50000);
	}

	return 0;
}
