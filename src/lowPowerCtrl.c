/*
 * lowPowerCtrl.c
 * 低功耗控制
 *  Created on: 2017年12月14日
 *      Author: hxbao
 */

#include "stm32f10x.h"
#include "diag/trace.h"

extern void sys_clock_init();

#define LDO_EN_PIN  GPIO_Pin_4
#define LDO_EN_PORT GPIOA

uint8_t low_ctrl_en = 1;

void disable_low_power()
{
	low_ctrl_en = 0;
}

void enable_low_power()
{
	low_ctrl_en = 1;
}

void entry_low_power()
{
	if(low_ctrl_en)
	{
#ifdef DEBUG
		trace_printf("enter save power mode\n");
#endif
		//close ldo en
		//GPIO_SetBits(LDO_EN_PORT,LDO_EN_PIN);
		//rola_m0m1_set(2);//rola 模块省电模式
		//使能wake pin 功能,需要开启pwr 控制寄存器时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
		//或许需要上升沿唤醒mcu，可以通过树莓派多发一次命令，加入唤醒命令
		PWR_WakeUpPinCmd(ENABLE);

		PWR_EnterSTANDBYMode();
	}
}

void exit_low_power()
{
	//open hse
	sys_clock_init();
	//open adc

}



