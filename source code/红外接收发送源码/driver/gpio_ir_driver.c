/*
 * gpio_ir_driver.c
 *
 *  Created on: 2015-4-28
 *      Author: robinson
 * Description: This driver is written for js9331 development board. You can use it
 * 				to send and receive ir signal
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
#include <linux/kfifo.h>


#include "gpio_ir_driver.h"

static ts_gpio_ir s_gpio_ir;

static bool eq_margin(unsigned d1, unsigned d2, unsigned margin)
{
        return ((d1 >= (d2 - margin)) && (d1 <= (d2 + margin)));
}

static int nec_decode_bit(unsigned char pulse_time, unsigned char space_time)
{
	int bit;
	if (eq_margin(pulse_time, GPIO_IR_NEC_BIT_PULSE, 2)){
		if (eq_margin(space_time, GPIO_IR_NEC_BIT_0_SPACE, 2)){
			bit = 0;
		}else if (eq_margin(space_time, GPIO_IR_NEC_BIT_1_SPACE, 2)){
			bit = 1;
		}else{
			printk("***Decode nec bit error 1! space_time:%d***\n", space_time);
			return -1;
		}
	}else{
		printk("***Decode nec bit error 2! pulse_time:%d***\n", pulse_time);
		return -1;
	}
	return bit;
}

static unsigned int nec_decode(unsigned char *sample_record)
{
	unsigned int code;
	unsigned char *code_record;
//	printk("***nec_decode***\n");
	code_record = sample_record + 1;
	int i;
	int bit;
	for (i = 0; i < 32; i++){
		bit = nec_decode_bit(*(code_record + i * 2), *(code_record + (i * 2) +1));
//		printk("code_record+%d:pulse_time:%d,space_time:%d\n",
//				i*2, *(code_record + i * 2), *(code_record + (i * 2) +1));
//		printk("Decode bit:%d\n",bit);
		if (bit != -1){
			code >>= 1;
			code |= (bit << 31);
		}else{
			return 0;
		}
	}
//	printk("***Get code: 0x%08X***\n",code);
//	printk("***address:0x%02X***\n", (code >> 0) & 0xFF);
//	printk("***not_address:0x%02X***\n", (code >> 8) & 0xFF);
//	printk("***command:0x%02X***\n", (code >> 16) & 0xFF);
//	printk("***not_command:0x%02X***\n",(code >> 24) & 0xFF);

	return code;
}

#if 0
static void set_ktime(ktime_t *pktime, unsigned long sample_period)
{
	unsigned long tmp_ns_time;
	unsigned int	tmp_s_time;
	tmp_s_time 	= sample_period / 1000000;
	tmp_ns_time = (sample_period % 1000000) * 1000;
	*pktime = ktime_set(tmp_s_time, tmp_ns_time);
//	printk("s_gpio_ir.sample_period:%d us\n", sample_period);
//	printk("s_gpio_ir.sample_period/1000000:%d s\n", tmp_s_time);
//	printk("tmp_ns_time:%d ns\n", tmp_ns_time);
}
#endif
#if 0
static int data_to_nec_encode(unsigned int *data, ts_nec_bit s_nec_bit[])
{
	int i;
	s_nec_bit[0].is_pulse 	= 1;
	s_nec_bit[0].periods 	= SEND_NEC_HEDER_PULSE_COUNTER;
	s_nec_bit[1].is_pulse 	= 0;
	s_nec_bit[1].periods 	= SEND_NEC_HEDER_SPACE_COUNTER;
	for (i = 0; i < 32; i++){
		if (data & (0x01 << i)){
			s_nec_bit[i+2]
		}
	}
	return 0;
}
#endif

static enum hrtimer_restart send_timer_handler(struct hrtimer *timer)
{
	static unsigned int 	send_data;
	static unsigned int 	pulse_counter;
	static int 			gpio_level;
	static unsigned char send_cod_pos;
	static unsigned char data_pulse_counter;
	static unsigned int	data_space_counter;
	int result;
	switch (s_gpio_ir.e_ir_send_status){
	case SEND_NEC_START:
//		printk("SEND_NEC_START\n");
		if (!kfifo_out(&(s_gpio_ir.ir_send_fifo), &send_data, sizeof(unsigned int))){
			//send over
			s_gpio_ir.e_ir_send_status = SEND_NEC_STOPED;
			printk("***ir_send_fifo is empty,result:%d***\n", result);
			goto out_send_stop;
		}else{
			s_gpio_ir.e_ir_send_status = SEND_NEC_HEADER_PULSE;
//			printk("SEND_NEC_HEADER_PULSE\n");
			s_gpio_ir.k_send_period = ktime_set(0, GPIO_IR_SEND_PERIOD);
			pulse_counter = 0;
			gpio_level = 0;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
		}
		break;
	case SEND_NEC_HEADER_PULSE:

//		if (pulse_counter >= SEND_NEC_HEDER_PULSE_COUNTER){//reach 9ms
		if (pulse_counter >= SEND_NEC_HEDER_PULSE_COUNTER){//reach 500ms
			s_gpio_ir.e_ir_send_status = SEND_NEC_HEADER_SPACE;
//			printk("SEND_NEC_HEADER_SPACE\n");
			gpio_level = 0;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
			pulse_counter = 0;
		}else{
			pulse_counter++;
			gpio_level = ~gpio_level;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
		}
		break;
	case SEND_NEC_HEADER_SPACE:

		if (pulse_counter >= SEND_NEC_HEDER_SPACE_COUNTER){//reach 4.5ms
			s_gpio_ir.e_ir_send_status = SEND_NEC_DATA;
//			printk("SEND_NEC_DATA\n");
			pulse_counter = 0;
			send_cod_pos = 0;
		}else{
			pulse_counter++;
		}
		break;
	case SEND_NEC_DATA:
		if (send_cod_pos < 32){
			if (send_data & (0x01 << send_cod_pos)){
				//send nec 1
				data_space_counter = SEND_NEC_DATA_1_SPACE_COUNTER;
			}else{
				//send nec 0
				data_space_counter = SEND_NEC_DATA_0_SPACE_COUNTER;
			}
			send_cod_pos++;
			s_gpio_ir.e_ir_send_status = SEND_NEC_DATA_PULSE;
//			printk("SEND_NEC_DATA_PULSE\n");
			data_pulse_counter = 0;
		}else{
			data_pulse_counter = 0;
			s_gpio_ir.e_ir_send_status = SEND_END_PULSE;
		}
		break;
	case SEND_NEC_DATA_PULSE:
		if (data_pulse_counter < SEND_NEC_DATA_PULSE_COUNTER){
			data_pulse_counter++;
			gpio_level = ~gpio_level;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
		}else{
			gpio_level = 0;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
			s_gpio_ir.e_ir_send_status = SEND_NEC_DATA_SPACE;
//			printk("SEND_NEC_DATA_SPACE\n");
		}
		break;
	case SEND_NEC_DATA_SPACE:
		if (data_space_counter){
			data_space_counter--;
		}else{
			s_gpio_ir.e_ir_send_status = SEND_NEC_DATA;
//			printk("SEND_NEC_DATA\n");
		}
		break;
	case SEND_END_PULSE:
		if (data_pulse_counter < SEND_NEC_DATA_PULSE_COUNTER){
			data_pulse_counter++;
			gpio_level = ~gpio_level;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
		}else{
			gpio_level = 0;
			gpio_set_value(s_gpio_ir.gpio_ir_out_pin, gpio_level);
			s_gpio_ir.e_ir_send_status = SEND_NEC_STOPED;
			goto out_send_stop;
		}
		break;
	default:
//		printk("###Unknown status!!###\n");
		break;
	}
	hrtimer_forward(&(s_gpio_ir.send_timer),
		s_gpio_ir.send_timer.base->get_time(),
		s_gpio_ir.k_send_period);
	return HRTIMER_RESTART;

out_send_stop:
	if (s_gpio_ir.e_ir_rcv_status == IR_RCV_PAUSE){//if receive pause before, turn on it
		s_gpio_ir.e_ir_rcv_status 		= IR_RCV_ON;
		s_gpio_ir.k_sample_period 		= ktime_set(0, GPIO_IR_IDLE_SAMPLE_PERIOD);
		s_gpio_ir.e_ir_rcv_sample_status= IDLE_SAMPLE_PERIOD_STATUS;
		hrtimer_start(&(s_gpio_ir.rcv_timer), s_gpio_ir.k_sample_period, HRTIMER_MODE_REL);
	}
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart rcv_timer_handler(struct hrtimer *timer)
{
	unsigned char 				sample_level;
	static unsigned char 		pre_sample_level;
	static unsigned char		period_counter;
	static unsigned short		sample_record_counter;
	static	unsigned char		sample_record[GPIO_IR_NEC_SAMPLE_RECORD_LENGTH];
	static te_rcv_nec_status e_rcv_nec_status;
//	printk("***hrtimer_handler***\n");
	switch (s_gpio_ir.e_ir_rcv_status){
	case IR_RCV_ON:
//		printk("**pwm_port_value == 1**\n");
		sample_level = gpio_get_value(s_gpio_ir.gpio_ir_in_pin);
		switch (s_gpio_ir.e_ir_rcv_sample_status){
		case IDLE_SAMPLE_PERIOD_STATUS:
			if (!sample_level){
				////detect logic 0, enter work status
				s_gpio_ir.e_ir_rcv_sample_status 	= WORK_SAMPLE_PERIOD_STATUS;
				e_rcv_nec_status					= RCV_NEC_HEADER_PULSE_STAUS;
	//			printk("***Enter WORK_SAMPLE_PERIOD***\n");
				s_gpio_ir.k_sample_period = ktime_set(0, GPIO_IR_WORK_SAMPLE_PERIOD);
				memset(&(sample_record), 0, sizeof(sample_record));
				sample_record_counter	= 0;
				pre_sample_level 		= 1;
				period_counter			= 0;
	//			printk("***test1***\n");
			}
			break;
		case WORK_SAMPLE_PERIOD_STATUS:
			//in work status

			switch (e_rcv_nec_status){
			case RCV_NEC_HEADER_PULSE_STAUS:
				period_counter++;
				if (pre_sample_level != sample_level){//pin logic has changed
					pre_sample_level = sample_level;
					sample_record[sample_record_counter] = period_counter;

					if (sample_record_counter == 2){//detect header space type
						if (eq_margin(period_counter, GPIO_IR_NEC_HEADER_SPACE, 2)){
							e_rcv_nec_status = RCV_NEC_NOMAL_DATA_STAUS;
						}else if (eq_margin(period_counter, GPIO_IR_NEC_REPEAT_SPACE, 2)){
							e_rcv_nec_status = RCV_NEC_REPEAT_DATA_STAUS;
						}else{
							//restart
							s_gpio_ir.e_ir_rcv_sample_status = IDLE_SAMPLE_PERIOD_STATUS;
							s_gpio_ir.k_sample_period = ktime_set(0, GPIO_IR_IDLE_SAMPLE_PERIOD);
							printk("***Unknown space:%d***\n", period_counter);
						}
					}

					sample_record_counter++;
					period_counter = 0;
				}
				break;
			case RCV_NEC_NOMAL_DATA_STAUS:
				period_counter++;
				if (pre_sample_level != sample_level){//pin level has been changed
					pre_sample_level = sample_level;
					sample_record[sample_record_counter] = period_counter;
					sample_record_counter++;
					period_counter = 0;
				}

				if (sample_record_counter >= GPIO_IR_NEC_SAMPLE_RECORD_LENGTH){
					//	printk("1\n");
					ts_ir_rcv_code s_ir_rcv_code;
					unsigned int code;
					//record completed
					s_gpio_ir.k_sample_period = ktime_set(0, GPIO_IR_IDLE_SAMPLE_PERIOD);
					s_gpio_ir.e_ir_rcv_sample_status = IDLE_SAMPLE_PERIOD_STATUS;
	//				for (i = 0; i < GPIO_IR_NEC_SAMPLE_RECORD_LENGTH; i++){
	//					printk("s_gpio_ir.sample_record[%d]:%d\n", i, sample_record[i]);
	//				}
					code = nec_decode(sample_record + 2);//skip two record
					if (code){//get correct code
						s_ir_rcv_code.code_data = code;
						s_ir_rcv_code.code_type = NEC_CODE_TYPE_NOMAL;
						if (kfifo_is_full(&(s_gpio_ir.ir_rcv_fifo))){//abandon the first code
							kfifo_out(&(s_gpio_ir.ir_rcv_fifo), NULL, sizeof(ts_ir_rcv_code));
						}
						kfifo_in(&(s_gpio_ir.ir_rcv_fifo), &s_ir_rcv_code, sizeof(ts_ir_rcv_code));
					}
//					printk("***ir_rcv_fifo length:%d***\n", kfifo_len(&(s_gpio_ir.ir_rcv_fifo)));
#if 0
					if (i > 10){
						while(kfifo_out(&(s_gpio_ir.ir_rcv_fifo), &s_ir_rcv_code, sizeof(ts_ir_rcv_code))){
							printk("GET code_data:0x%08X\n", s_ir_rcv_code.code_data);
							printk("GET code_type:0x%02X\n", s_ir_rcv_code.code_type);
						}
						i = 0;
					}else{
						i++;
					}
#endif
				}

				break;
			case RCV_NEC_REPEAT_DATA_STAUS:{
//				printk("2\n");
				ts_ir_rcv_code s_ir_rcv_code;
				s_gpio_ir.k_sample_period = ktime_set(0, GPIO_IR_IDLE_SAMPLE_PERIOD);
				s_gpio_ir.e_ir_rcv_sample_status = IDLE_SAMPLE_PERIOD_STATUS;
				s_ir_rcv_code.code_data = 0;
				s_ir_rcv_code.code_type = NEC_CODE_TYPE_REPEAT;
				if (kfifo_is_full(&(s_gpio_ir.ir_rcv_fifo))){//abandon the first code
					kfifo_out(&(s_gpio_ir.ir_rcv_fifo), NULL, sizeof(ts_ir_rcv_code));
				}
				kfifo_in(&(s_gpio_ir.ir_rcv_fifo), &s_ir_rcv_code, sizeof(ts_ir_rcv_code));
//				printk("***ir_rcv_fifo length:%d***\n", kfifo_len(&(s_gpio_ir.ir_rcv_fifo)));
				break;
			}

			default:

				break;
		}


#if 0
			int i;
			if (tmp >= 1000){
				set_ktime(&(s_gpio_ir.k_sample_period), GPIO_IR_IDLE_SAMPLE_PERIOD);
				s_gpio_ir.e_ir_rcv_sample_status = IDLE_SAMPLE_PERIOD;
				for (i = 0; i < 1000; i++){
					printk("tmp_record[%d]:%d\n", i, tmp_record[i]);
				}
				tmp = 0;
			}else{
				tmp_record[tmp] = sample_level;
				tmp++;
			}
#endif
		break;
		}

		hrtimer_forward(&(s_gpio_ir.rcv_timer),
			s_gpio_ir.rcv_timer.base->get_time(),
			s_gpio_ir.k_sample_period);
		break;
	case IR_RCV_OFF://will not excuse, because timer has stopped
	case IR_RCV_PAUSE://
		printk("***case IR_RCV_PAUSE***\n");
		return HRTIMER_NORESTART;//not restart again
		break;
	default:
		printk("###Unknown Reiceive Status!!###\n");
		break;
	}

	return HRTIMER_RESTART;
}

//used for module parameter
static int gpio_ir_out_pin 	= -1;
static int gpio_ir_in_pin		= -1;

static int gpio_ir_open(struct inode *pinode, struct file *pfile)
{
	int result;
	printk("***%s***\n",__func__);
	//initialize
	s_gpio_ir.e_ir_rcv_status		= IR_RCV_OFF;

	if (gpio_ir_out_pin == -1){
		s_gpio_ir.gpio_ir_out_pin 	= GPIO_IR_DEF_OUT_PIN;
	}else{
		s_gpio_ir.gpio_ir_out_pin	= gpio_ir_out_pin;
	}
	result = gpio_request(s_gpio_ir.gpio_ir_out_pin, GPIO_IR_OUT_PIN_NMAE);
	if (result < 0){
		printk("###gpio_request ERROR: can't request %d pin for output###\n",
				s_gpio_ir.gpio_ir_out_pin);
		return -1;
	}

	if (gpio_ir_in_pin == -1){//use default values, if no module parameter
		s_gpio_ir.gpio_ir_in_pin 	= GPIO_IR_DEF_IN_PIN;
	}else{
		s_gpio_ir.gpio_ir_in_pin	= gpio_ir_in_pin;
	}
	result = gpio_request(s_gpio_ir.gpio_ir_in_pin, GPIO_IR_IN_PIN_NMAE);
	if (result < 0){
		printk("###gpio_request ERROR: can't request %d pin for input###\n",
				s_gpio_ir.gpio_ir_in_pin);
		return -1;
	}
	gpio_direction_input(s_gpio_ir.gpio_ir_in_pin);

	if (kfifo_alloc(&(s_gpio_ir.ir_rcv_fifo),
			sizeof(ts_ir_rcv_code) * GPIO_IR_RCV_FIFO_LENGTH, GFP_KERNEL)){
		printk("###kfifo_alloc ir_rcv_fifo ERROR###\n");
		return -1;
	}

	if (kfifo_alloc(&(s_gpio_ir.ir_send_fifo),
			sizeof(unsigned int) * GPIO_IR_SEND_FIFO_LENGTH, GFP_KERNEL)){
		printk("###kfifo_alloc ir_send_fifo ERROR###\n");
		return -1;
	}
	gpio_direction_output(s_gpio_ir.gpio_ir_out_pin, 0);

//	printk("***send_fifo size:%d***\n",kfifo_size(&(s_gpio_ir.ir_send_fifo)));
//	printk("***rcv_fifo size:%d***\n",kfifo_size(&(s_gpio_ir.ir_rcv_fifo)));
    printk("**gpio_ir_out_pin:%d**\n",s_gpio_ir.gpio_ir_out_pin);
    printk("**gpio_ir_in_pin:%d**\n",s_gpio_ir.gpio_ir_in_pin);

	//initialize timer ,and start now
	hrtimer_init(&(s_gpio_ir.send_timer), CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	s_gpio_ir.send_timer.function = send_timer_handler;
	hrtimer_init(&(s_gpio_ir.rcv_timer), CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	s_gpio_ir.rcv_timer.function = rcv_timer_handler;

	return 0;
}

static int gpio_ir_release(struct inode *pinode, struct file *pfile)
{
	printk("***%s***\n",__func__);
	/*
	 * delete timer, to prevent a unexpected exit of the pwm app,
	 *   if we not call this fuc, it may cause " Fatal exception in interrupt"
	 *   then  system reboot
	 */
	hrtimer_cancel(&(s_gpio_ir.rcv_timer));//delete timer
	hrtimer_cancel(&(s_gpio_ir.send_timer));//delete timer
	//free the gpio that we requested before
	gpio_direction_input(s_gpio_ir.gpio_ir_out_pin);
	gpio_direction_input(s_gpio_ir.gpio_ir_in_pin);
	gpio_free(s_gpio_ir.gpio_ir_in_pin);
	gpio_free(s_gpio_ir.gpio_ir_out_pin);

	kfifo_free(&(s_gpio_ir.ir_send_fifo));
	kfifo_free(&(s_gpio_ir.ir_rcv_fifo));
	return 0;
}

static ssize_t gpio_ir_read(struct file *pfile, char __user *pbuf, size_t size, loff_t *ppos)
{
	int result = 0;
	if (!kfifo_is_empty(&(s_gpio_ir.ir_rcv_fifo))){
		result = kfifo_out(&(s_gpio_ir.ir_rcv_fifo), pbuf, sizeof(ts_ir_rcv_code));
//		printk("***sizeof(ts_ir_rcv_code):%d***\n", sizeof(ts_ir_rcv_code));
//		printk("***result:%d***\n", result);
	}
	return result;
}

static ssize_t gpio_ir_write(struct file *pfile, const char __user *pbuf, size_t size, loff_t *ppos)
{
	int result = 0;
	unsigned int send_code;
	unsigned int tmp_code;
	if (s_gpio_ir.e_ir_rcv_status == IR_RCV_ON){//if receive on, pause it until send complete
		s_gpio_ir.e_ir_rcv_status = IR_RCV_PAUSE;
		hrtimer_cancel(&(s_gpio_ir.rcv_timer));//delete timer
	}

//	static int i;
	send_code = *((unsigned int *)pbuf);
//	printk("***get nec code_data:0x%08X***\n", send_code);
//	printk("***address:0x%02X***\n", (send_code >> 0) & 0xFF);
//	printk("***not_address:0x%02X***\n", (send_code >> 8) & 0xFF);
//	printk("***command:0x%02X***\n", (send_code >> 16) & 0xFF);
//	printk("***not_command:0x%02X***\n",(send_code >> 24) & 0xFF);

//	printk("***ir_rcv_fifo length:%d***\n", kfifo_len(&(s_gpio_ir.ir_send_fifo)));
	if (kfifo_is_full(&(s_gpio_ir.ir_send_fifo))){//abandon the first
		kfifo_out(&(s_gpio_ir.ir_send_fifo), &tmp_code, sizeof(unsigned int));
	}
	result = kfifo_in(&(s_gpio_ir.ir_send_fifo), &send_code, sizeof(unsigned int));
//	printk("***result:%d***\n", result);
	if (s_gpio_ir.e_ir_send_status == SEND_NEC_STOPED){
		s_gpio_ir.e_ir_send_status = SEND_NEC_START;
		hrtimer_start(&(s_gpio_ir.send_timer), ktime_set(0,1000), HRTIMER_MODE_REL);//timer expire now
	}

#if 0
	if (i > 20){
		i = 0;
		while(kfifo_out(&(s_gpio_ir.ir_send_fifo), &tmp_code, sizeof(unsigned int))){
			printk("***GET 0x%08X***\n", tmp_code);
		}
	}else{
		i++;
	}
#endif
	return result;
}

static long gpio_ir_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
	printk("***%s***\n",__func__);
	switch (cmd){
	case GPIO_IR_START_RCV:
		printk("***GPIO_IR_START_RCV***\n");
		if (s_gpio_ir.e_ir_rcv_status == IR_RCV_OFF){
			s_gpio_ir.k_sample_period 		= ktime_set(0, GPIO_IR_IDLE_SAMPLE_PERIOD);
			s_gpio_ir.e_ir_rcv_sample_status= IDLE_SAMPLE_PERIOD_STATUS;
			s_gpio_ir.e_ir_rcv_status 		= IR_RCV_ON;
			hrtimer_start(&(s_gpio_ir.rcv_timer), ktime_set(0,0), HRTIMER_MODE_REL);//start timer,and expire now
		}else{
					printk("**IR receive already on!!**\n");
		}
		break;
	case GPIO_IR_STOP_RCV:
			printk("**GPIO_IR_STOP_RCV**\n");
			if (s_gpio_ir.e_ir_rcv_status == IR_RCV_ON){
				s_gpio_ir.e_ir_rcv_status = IR_RCV_OFF;
				hrtimer_cancel(&(s_gpio_ir.rcv_timer));//delete timer
			}else{
				printk("**IR receive already off!!**\n");
			}
		break;
	default:
		printk("###Unknown command: magic 0x%02X, number %d###\n",
				(int)_IOC_TYPE(cmd),(int)_IOC_NR(cmd));
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations gpio_ir_ops = {
		.owner 			= THIS_MODULE,
		.open			= gpio_ir_open,
		.release		= gpio_ir_release,
		.unlocked_ioctl	= gpio_ir_ioctl,
		.read			= gpio_ir_read,
		.write			= gpio_ir_write
};


//module initialize function
static int gpio_ir_init(void)
{
	int result;
	//initialize

	//request device number
	s_gpio_ir.devno = MKDEV(GPIO_IR_MAJOR,0);
	result = register_chrdev_region(s_gpio_ir.devno, 1, "gpio_ir");
	if (result < 0){
		printk("##register_chrdev_region:failed##\n");
		return -1;
	}
	//initialize and register device
	cdev_init(&(s_gpio_ir.s_cdev),&gpio_ir_ops);
	s_gpio_ir.s_cdev.owner = THIS_MODULE;
	result = cdev_add(&(s_gpio_ir.s_cdev), s_gpio_ir.devno, 1);
    if(result)
    {
        printk("##cdev_add:Error %d adding cdev\n##",result);
        return -1;
    }

	//create class
    s_gpio_ir.p_gpio_ir_dev_class = class_create(THIS_MODULE, GPIO_IR_DEV_NAME);
	if (IS_ERR(s_gpio_ir.p_gpio_ir_dev_class)){
		printk("###class_create:failed to create class!###\n");
		return -1;
	}

	//create node
	device_create(s_gpio_ir.p_gpio_ir_dev_class, NULL,
			s_gpio_ir.devno, NULL, GPIO_IR_DEV_NAME);

    printk("**gpio_ir module initiation OK**\n");

	return 0;
}

//module exit fuc
void gpio_ir_exit(void)
{
	//unregister what we registered
	device_destroy(s_gpio_ir.p_gpio_ir_dev_class, s_gpio_ir.devno);
	class_destroy(s_gpio_ir.p_gpio_ir_dev_class);
	cdev_del(&(s_gpio_ir.s_cdev));
	unregister_chrdev_region(MKDEV(GPIO_IR_MAJOR,0),1);

	printk("**gpio_ir module exit**\n");
}

module_init(gpio_ir_init);
module_exit(gpio_ir_exit);

module_param(gpio_ir_out_pin, int, S_IRUGO);
MODULE_PARM_DESC(gpio_ir_out_pin,"gpio number for ir output, defualt is 0");

module_param(gpio_ir_in_pin, int, S_IRUGO);
MODULE_PARM_DESC(gpio_ir_in_pin,"gpio number for ir input, defualt is 12");

MODULE_VERSION("V1.0");
MODULE_AUTHOR("Robinson wu <wurobinson@joysince.com>");
MODULE_LICENSE("Dual BSD/GPL");
