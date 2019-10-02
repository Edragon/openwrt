#ifndef GPIO_CONTROL_APP_H_
#define GPIO_CONTROL_APP_H_

#define GPIO_CONTROL_DEVICE_PATH		"/dev/gpio_control"

#define GPIO_IOCTL_PRAM(gpio_num, arg1) (((unsigned long)gpio_num << 24) + ((unsigned long)arg1 << 16))
#define GET_GPIO_NUM(arg1) (unsigned char)((arg1 >> 24) & 0xff)
#define GET_GPIO_VALUE(arg1) (unsigned char)((arg1 >> 16) & 0xff)

//IOCTRL CMDs
#define GPIO_CONTROL_SET_OUT			0x01
#define GPIO_CONTROL_SET_IN			0x02
//#define GPIO_CONTROL_GET_DIRECTION		0x03
#define GPIO_CONTROL_SET_VALUE			0x04
#define GPIO_CONTROL_GET_VALUE			0x05
#define GPIO_CONTROL_REQUEST_GPIO		0x06
#define GPIO_CONTROL_FREE_GPIO			0x07

#endif
