/*
 * gpio_ir_app.h
 *
 *  Created on: 2015-5-10
 *      Author: robinson
 */

#ifndef GPIO_IR_APP_H_
#define GPIO_IR_APP_H_

#define GPIO_IR_DEVICE_PATH		"/dev/js9331_gpio_ir"
#define GPIO_IR_FIFO_PATH			"/tmp/gpio_ir_fifo"

#define MAX_RCV_BUF_SIZE			512//Byte
#define MAX_BUF_SIZE				128//Byte

#define GPIO_IR_MAGIC 				'S'
#define GPIO_IR_START_RCV	_IO(GPIO_IR_MAGIC,0)
#define GPIO_IR_STOP_RCV	_IO(GPIO_IR_MAGIC,1)

#define NEC_CODE_TYPE_NOMAL 	0x01
#define NEC_CODE_TYPE_REPEAT  	0x02

typedef struct{//gpio_ir_app class
	//public

	//private
	int		dev_fd;
	char	dev_path[256];
	int 	fifo_fd;//file description of the fifo file
}ts_gpio_ir_app;

typedef struct{
	unsigned char code_type;
	unsigned int	code_data;
}ts_ir_rcv_code;


#endif /* GPIO_IR_APP_H_ */
