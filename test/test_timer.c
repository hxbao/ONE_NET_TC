/*
 * test_timer.c
 *
 *  Created on: 2017年12月1日
 *      Author: hxbao
 */


#include "test.h"

uint8_t gpio_pc2_state = 0;

void test_timer_call_fun()
{
	static uint16_t i =0;

	if(gpio_pc2_state == 0)
	{
		gpio_pc2_state = 1;
		GPIO_SetBits(GPIOC,GPIO_Pin_2);
	}
	else
	{
		gpio_pc2_state = 0;
		GPIO_ResetBits(GPIOC,GPIO_Pin_2);
	}
#ifdef DEBUG
			trace_printf("Test timer:call count %d\n", i++);
#endif
}

void test_timer()
{
	GPIO_InitTypeDef gpio;
	//初始化一个GPIO管脚
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC,ENABLE);
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_2; //lora M0 ,pc2
	gpio.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC,&gpio);




	timerhw_create(10000,1,test_timer_call_fun);
	timerhw_start();
	while(1);
}


