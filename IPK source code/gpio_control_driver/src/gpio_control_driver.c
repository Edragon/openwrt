/*
 *  Gpio control driver
 *
 *  Copyright (C) 2017 (C) <wurobinson@zhuotk.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * 
 * Description: This driver is written for JS9331 and JS7628 development board. You can use it
 * 		to control gpios
 *
 */

#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/hrtimer.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <asm-generic/errno-base.h>
#include <linux/miscdevice.h>


#include "gpio_control_driver.h"

static int gpio_control_open(struct inode *pinode, struct file *pfile)
{
	printk("***%s***\n",__func__);
	//initialize

	return 0;
}

static int gpio_control_release(struct inode *pinode, struct file *pfile)
{
	printk("***%s***\n",__func__);

	return 0;
}


static long gpio_control_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int ret;
	unsigned char gpio_number;
	unsigned char gpio_value;

	//printk("***%s***\n",__func__);
	//printk("cmd:0x%02X\n", cmd);

	gpio_number = GET_GPIO_NUM(arg);
	gpio_value  = GET_GPIO_VALUE(arg);
	//printk("gpio number:%d\n", gpio_number);
	//printk("gpio value:0x%02X\n", gpio_value);

	switch (cmd){
	case GPIO_CONTROL_SET_OUT:
		//printk("command: GPIO_CONTROL_SET_OUT\n");
		ret = gpio_direction_output(gpio_number, gpio_value);
		if (ret < 0){
			//printk("###gpio_direction_output ERROR: can't set gpio %d output###\n", gpio_number);
			return -1;
		}
		//printk("command: GPIO_CONTROL_SET_OUT done\n");
		break;

	case GPIO_CONTROL_SET_IN:
		ret = gpio_direction_input(gpio_number);
		if (ret < 0){
			//printk("###gpio_direction_input ERROR: can't set gpio %d input###\n", gpio_number);
			return -1;
		}
		//printk("command: GPIO_CONTROL_SET_IN\n");
		break;
#if 0
	case GPIO_CONTROL_GET_DIRECTION:

		printk("command: GPIO_CONTROL_GET_DIRECTION\n");
		break;
#endif
	case GPIO_CONTROL_SET_VALUE:
		gpio_set_value(gpio_number, gpio_value);
		//printk("command: GPIO_CONTROL_SET_VALUE\n");
		break;

	case GPIO_CONTROL_GET_VALUE:
		ret = gpio_get_value(gpio_number);
		if (ret < 0){
			//printk("###gpio_get_value ERROR: can't get gpio %d value###\n", gpio_number);
			return -1;
		}
		//printk("command: GPIO_CONTROL_GET_VALUE\n");
		break;

	case GPIO_CONTROL_REQUEST_GPIO:
		//printk("command: GPIO_CONTROL_REQUEST_ONE\n");
		if (0 > gpio_request(gpio_number, "gpio_ctrl")){
			//printk("###gpio_request ERROR: can't request %d pin for output###\n", gpio_number);
			return -1;
		}
		//printk("command: GPIO_CONTROL_REQUEST_GPIO done\n");
		break;

	case GPIO_CONTROL_FREE_GPIO:
		gpio_free(gpio_number);
		//printk("command: GPIO_CONTROL_FREE_GPIO done\n");
		break;

	default:
		printk("***Unknown command:0x%02X\n***\n", cmd);
		break;

	}

	return 0;
}

static const struct file_operations gpio_control_ops = {
		.owner 			= THIS_MODULE,
		.open			= gpio_control_open,
		.release		= gpio_control_release,
		.unlocked_ioctl	= gpio_control_ioctl,
};

static struct miscdevice s_gpio_control_dev = {
		.minor = MISC_DYNAMIC_MINOR,
		.fops = &gpio_control_ops,
		.name = GPIO_CONTROL_DEV_NAME
};

//module initialize function
static int gpio_control_init(void)
{
	int result;
	//initialize and register device
	result = misc_register(&s_gpio_control_dev);
	if (result != 0) {
		//printk("###misc_register error###\n");
		return -1;
	}

	printk("**gpio_control module initiation OK**\n");
	return result;
}

//module exit fuc
void gpio_control_exit(void)
{
	//unregister what we registered
	misc_deregister(&s_gpio_control_dev);

	printk("**gpio_control module exit**\n");
}

module_init(gpio_control_init);
module_exit(gpio_control_exit);

MODULE_VERSION("V1.0");
MODULE_AUTHOR("wurobinson <wurobinson@zhuotk.com>");
MODULE_LICENSE("Dual BSD/GPL");
