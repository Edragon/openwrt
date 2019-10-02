/*
 * gpio_control_app
 *
 * Copyright (C) 2017 wurobinson <wurobinson@zhuotk.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is written for JS7628 and JS9331 development board ,
 * work with gpio_control_driver
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>

#include "gpio_control_app.h"

static unsigned char gpio_pin;//define GPIOs to be use
static int gpio_dev_fd;

void demo1_release(int signal_no)
{
	ioctl(gpio_dev_fd, GPIO_CONTROL_SET_IN, GPIO_IOCTL_PRAM(gpio_pin, 0));
	ioctl(gpio_dev_fd, GPIO_CONTROL_FREE_GPIO, GPIO_IOCTL_PRAM(gpio_pin, 0));
	exit(0);
}

int main(int argc, char *argv[])
{
	int ret;
	int i;

	if (argc == 1){
	//main process
		i = 0;
		while(1){
			printf("gpio_control_app:%d\n", i);
			i++;
			if (i >= 10) i = 0;

			//TODO

			sleep(1);
		}
	}else if (argc >= 2){
	/*demo*/
		if(0 == strcmp(argv[1],"demo1")){
			gpio_dev_fd = open(GPIO_CONTROL_DEVICE_PATH, O_RDWR);//open gpio device
			if (gpio_dev_fd < 0){
				printf("###open %s ERROR###\n", GPIO_CONTROL_DEVICE_PATH);
				return -1;
			}else{
				printf("***open %s success***\n", GPIO_CONTROL_DEVICE_PATH);
			}

			gpio_pin = (unsigned char)atoi(argv[2]);//get gpio pin
			printf("gpio_pin:%d\n", gpio_pin);

			ret = ioctl(gpio_dev_fd, GPIO_CONTROL_REQUEST_GPIO, GPIO_IOCTL_PRAM(gpio_pin, 0));
			if (ret < 0){
				printf("###request GPIO %d error###", gpio_pin);
				goto __error;
			}
			ret = ioctl(gpio_dev_fd, GPIO_CONTROL_SET_OUT, GPIO_IOCTL_PRAM(gpio_pin, 0));
			if (ret < 0){
				printf("###set GPIO %d output error###", gpio_pin);
				goto __error;
			}

			signal(SIGINT, demo1_release);//register terminal signal
			while(1){
				//turn gpio off then turn on gpio
				ioctl(gpio_dev_fd, GPIO_CONTROL_SET_VALUE, GPIO_IOCTL_PRAM(gpio_pin, 0));
				sleep(1);
				ioctl(gpio_dev_fd, GPIO_CONTROL_SET_VALUE, GPIO_IOCTL_PRAM(gpio_pin, 1));
				sleep(1);
			}

		}else if(0 == strcmp(argv[1],"-h")){
			printf("gpio_contrl_app usage: demo1 <GPIO number>\n");
		}else{
			printf("***Unknown command:%s***\n", argv[1]);
			printf("gpio_contrl_app usage: demo1 <GPIO number>\n");
		}
	}
	return 0;

__error:
	return -1;


}
