/*
 * gpio_ir.h
 *
 *  Created on: 2015-5-10
 *      Author: robinson
 */

#ifndef GPIO_IR_H_
#define GPIO_IR_H_

#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/kfifo.h>

#define GPIO_IR_MAJOR 					98//device major number

#define GPIO_IR_DEF_OUT_PIN			0
#define GPIO_IR_OUT_PIN_NMAE			"gpio_ir_out"
#define GPIO_IR_DEF_IN_PIN				12
#define GPIO_IR_IN_PIN_NMAE			"gpio_ir_in"

#define GPIO_IR_IDLE_SAMPLE_PERIOD		8000000//nanosecond
#define GPIO_IR_WORK_SAMPLE_PERIOD		140000//nanosecond,560000/4

#define GPIO_IR_SEND_PERIOD			13158//nanosecond,(1/38000)/2
#define SEND_NEC_HEDER_PULSE_COUNTER	684// 9000000/GPIO_IR_SEND_PERIOD
#define SEND_NEC_HEDER_SPACE_COUNTER	342// 4500000/GPIO_IR_SEND_PERIOD
#define SEND_NEC_DATA_PULSE_COUNTER	42// 560000/GPIO_IR_SEND_PERIOD
#define SEND_NEC_DATA_1_SPACE_COUNTER	128// 16900000/GPIO_IR_SEND_PERIOD
#define SEND_NEC_DATA_0_SPACE_COUNTER	42// 560000/GPIO_IR_SEND_PERIOD

#define GPIO_IR_NEC_UNIT               (560000/GPIO_IR_WORK_SAMPLE_PERIOD)
#define GPIO_IR_NEC_HEADER_SPACE		(8  * GPIO_IR_NEC_UNIT)
#define GPIO_IR_NEC_REPEAT_SPACE		(4  * GPIO_IR_NEC_UNIT)
#define GPIO_IR_NEC_BIT_PULSE			(1  * GPIO_IR_NEC_UNIT)
#define GPIO_IR_NEC_BIT_0_SPACE		(1  * GPIO_IR_NEC_UNIT)
#define GPIO_IR_NEC_BIT_1_SPACE		(3  * GPIO_IR_NEC_UNIT)


#define GPIO_IR_NEC_SAMPLE_RECORD_LENGTH	67
#define GPIO_IR_NEC_SEND_RECORD_LENGTH		66

#define GPIO_IR_RCV_FIFO_LENGTH		16
#define GPIO_IR_SEND_FIFO_LENGTH		16

#define GPIO_IR_DEV_NAME		"js9331_gpio_ir"

#define NEC_CODE_TYPE_NOMAL 	0x01
#define NEC_CODE_TYPE_REPEAT  	0x02

//IOCTRL CMDs
#define GPIO_IR_MAGIC 				'S'
#define GPIO_IR_START_RCV	_IO(GPIO_IR_MAGIC,0)
#define GPIO_IR_STOP_RCV	_IO(GPIO_IR_MAGIC,1)

typedef enum{
	IR_RCV_ON = 0,
	IR_RCV_PAUSE = 1,
	IR_RCV_OFF = 2,
}te_ir_rcv_status;

typedef enum{
	IDLE_SAMPLE_PERIOD_STATUS,
	WORK_SAMPLE_PERIOD_STATUS
}te_ir_rcv_sample_status;


typedef enum{
	RCV_NEC_HEADER_PULSE_STAUS,
	RCV_NEC_NOMAL_DATA_STAUS,
	RCV_NEC_REPEAT_DATA_STAUS
}te_rcv_nec_status;


typedef struct{
	unsigned char code_type;
	unsigned int	code_data;
}ts_ir_rcv_code;

typedef enum{
	SEND_NEC_STOPED,
	SEND_NEC_START,
	SEND_NEC_HEADER_PULSE,
	SEND_NEC_HEADER_SPACE,
	SEND_NEC_DATA,
	SEND_NEC_DATA_PULSE,
	SEND_NEC_DATA_SPACE,
	SEND_END_PULSE
}te_ir_send_status;

typedef struct {
	unsigned int 	periods;
	bool			is_pulse;
}ts_nec_bit;

typedef struct{//gpio ir class
	//public

	//private
	struct cdev				s_cdev;
	int						gpio_ir_out_pin;
	int						gpio_ir_in_pin;
	struct class			*p_gpio_ir_dev_class;
	dev_t 					devno;
	ktime_t					k_sample_period;			//DO NOT care, used by ktime
	ktime_t					k_send_period;
	struct 	hrtimer 		rcv_timer;					//timer for receiving
	struct 	hrtimer 		send_timer;					//timer for sending
	te_ir_rcv_status		e_ir_rcv_status;
	te_ir_send_status		e_ir_send_status;
	te_ir_rcv_sample_status	e_ir_rcv_sample_status;
	//fifo
	struct kfifo 			ir_rcv_fifo;
	struct kfifo 			ir_send_fifo;
}ts_gpio_ir;


#endif /* GPIO_IR_H_ */
