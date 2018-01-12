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

#ifndef TIMER_H_
#define TIMER_H_

#include "cmsis_device.h"

//定时器回调函数定义
typedef void (*pTimerCallFun)(void);

typedef struct
{
	uint8_t id;
	uint8_t timerMode; //定时器的工作模式，ONCE_TIME = 0，PERIODIC = 1
	uint32_t ms;      //定时的周期
	pTimerCallFun  pTimerFunCall;
}Timer_Def;

// ----------------------------------------------------------------------------

#define TIMER_FREQUENCY_HZ (1000u)

typedef uint32_t timer_ticks_t;

extern volatile timer_ticks_t timer_delayCount;

void timer_start(void);

void timer_sleep(timer_ticks_t ticks);

//创建一个硬件定时器
//Tout= ((arr+1)*(psc+1))/Tclk
//最大定时长32.768s，最小定时0.5ms
void timerhw_create(uint16_t ms,uint8_t mode,pTimerCallFun pfun);
//定时器开始计数
void timerhw_start(void);
void timerhw_stop();

void timer1hw_create(uint16_t ms,uint8_t mode,pTimerCallFun pfun);
//定时器开始计数
void timer1hw_start(void);
void timer1hw_stop();

// ----------------------------------------------------------------------------

#endif // TIMER_H_
