/*
 * This file is part of the 碌OS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <timer.h>
#include "cortexm/ExceptionHandlers.h"
#include "g_conf.h"

// ----------------------------------------------------------------------------

#if defined(USE_HAL_DRIVER)
void HAL_IncTick(void);
#endif

// Forward declarations.

void timer_tick(void);

// ----------------------------------------------------------------------------

volatile timer_ticks_t timer_delayCount;

// ----------------------------------------------------------------------------

void timer_start(void)
{
	// Use SysTick as reference for the delay loops.
	SysTick_Config(SystemCoreClock / TIMER_FREQUENCY_HZ);
}

void timer_sleep(timer_ticks_t ticks)
{
	timer_delayCount = ticks;

	// Busy wait until the SysTick decrements the counter to zero.
	while (timer_delayCount != 0u)
		;
}

void timer_tick(void)
{
	// Decrement to zero the counter used by the delay routine.
	if (timer_delayCount != 0u)
	{
		--timer_delayCount;
	}
}

// ----- SysTick_Handler() ----------------------------------------------------

void SysTick_Handler(void)
{
#if defined(USE_HAL_DRIVER)
	HAL_IncTick();
#endif
	timer_tick();
}

// ----------------------------------------------------------------------------


Timer_Def timer = {0,0,0,0};
//创建一个硬件定时器
////Tout= ((arr+1)*(psc+1))/Tclk
//apb1 ftclk = 72M
void timerhw_create(uint16_t ms,uint8_t mode,pTimerCallFun pfun)
{
	uint16_t reloadVal = ms*2;
	timer.ms = reloadVal;
	timer.timerMode = mode;
	timer.pTimerFunCall = pfun;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	TIM_DeInit(TIM3);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//①使能TIM3时钟

	TIM_TimeBaseInitStructure.TIM_Period = reloadVal;//自动重装载值
#ifndef USE_STM32L151
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36000-1;//定时器分频2khz
#elif
	TIM_TimeBaseInitStructure.TIM_Prescaler = 16000-1;//定时器分频2khz
#endif
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //4分频
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);//②初始化定时器TIM3

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//③允许定时器3更新中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;//定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INTER_TIM3;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//响应优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);//④初始化NVIC


}
//定时器开始计数
void timerhw_start()
{
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	TIM_Cmd(TIM3, ENABLE); //⑤使能定时器3
}

void timerhw_stop()
{
	TIM_Cmd(TIM3, DISABLE); //使能定时器3
}

//定时器服务中断函数
void TIM3_IRQHandler()
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) //溢出中断
	{
		//调用一次回调函数
		if(!timer.timerMode )//一次模式
		{

			TIM_Cmd(TIM3, DISABLE);
			(*timer.pTimerFunCall)();
		}else
		{
			(*timer.pTimerFunCall)();
		}

	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除中断标志位
}


Timer_Def timer1 = {0,0,0,0};
//创建一个硬件定时器
////Tout= ((arr+1)*(psc+1))/Tclk
//apb1 ftclk = 72M
void timer1hw_create(uint16_t ms,uint8_t mode,pTimerCallFun pfun)
{
	uint16_t reloadVal = ms*2;
	timer1.ms = reloadVal;
	timer1.timerMode = mode;
	timer1.pTimerFunCall = pfun;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_DeInit(TIM2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//①使能TIM3时钟

	TIM_TimeBaseInitStructure.TIM_Period = reloadVal;//自动重装载值
#ifndef USE_STM32L151
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36000-1;//定时器分频2khz
#elif
	TIM_TimeBaseInitStructure.TIM_Prescaler = 16000-1;//定时器分频2khz
#endif
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //4分频
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);//②初始化定时器TIM3

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);//③允许定时器3更新中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;//定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INTER_TIM2;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//响应优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);//④初始化NVIC
}
//定时器开始计数
void timer1hw_start()
{
	TIM_Cmd(TIM2, ENABLE); //⑤使能定时器3
}

void timer1hw_stop()
{
	TIM_Cmd(TIM2, DISABLE); //使能定时器3
}

//定时器服务中断函数
void TIM2_IRQHandler()
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) //溢出中断
	{
		//调用一次回调函数
		if(!timer1.timerMode )//一次模式
		{

			TIM_Cmd(TIM2, DISABLE);
			(*timer1.pTimerFunCall)();
		}else
		{
			(*timer1.pTimerFunCall)();
		}

	}
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除中断标志位
}

