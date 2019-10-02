/*
 * gpio_control_driver.h
 *
 *  Created on: 2017-9-10
 *      Author: robinson
 */

#ifndef GPIO_CONTROL_DRIVER_H_
#define GPIO_CONTROL_DRIVER_H_

#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/kfifo.h>

#define GET_GPIO_NUM(arg1) (unsigned char)((arg1 >> 24) & 0xff)
#define GET_GPIO_VALUE(arg1) (unsigned char)((arg1 >> 16) & 0xff)

#define GPIO_CONTROL_MAJOR 					99//device major number


#define GPIO_CONTROL_DEV_NAME		"gpio_control"

//IOCTRL CMDs
#define GPIO_CONTROL_SET_OUT			0x01
#define GPIO_CONTROL_SET_IN			0x02
//#define GPIO_CONTROL_GET_DIRECTION		0x03
#define GPIO_CONTROL_SET_VALUE			0x04
#define GPIO_CONTROL_GET_VALUE			0x05
#define GPIO_CONTROL_REQUEST_GPIO		0x06
#define GPIO_CONTROL_FREE_GPIO			0x07



#endif /* GPIO_CONTROL_DRIVER_H_ */
