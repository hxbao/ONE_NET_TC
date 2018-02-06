/*
 * modbus.c
 *
 *  Created on: 2018年1月9日
 *      Author: hxbao
 */

// ----------------------------------------------------------------------------
#include <mymisc.h>
#include <timer.h>
#include "diag/Trace.h"
#include "stm32f10x.h"
#include "commd_if.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "test.h"
#include "timer.h"
#include "lowPowerCtrl.h"
#include "m6311r.h"
#include "modbus.h"
#include "iwdg.h"
#include "SEGGER_RTT.h"

#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wmissing-declarations"
//#pragma GCC diagnostic ignored "-Wreturn-type"
uint32_t timeCallback = 0;

//50ms 轮询调度,中断中执行,其中不行调用有中断的函数
void task_poll_485()
{

	modbus_task();
}

//1s 轮询任务

void task_poll_1s()
{

#ifdef DEBUG
	//trace_printf("call task poll 1s\n");
#endif
	//poll_store_commd();
	//cmds和flash内容同步
	//store_cmd_para_to_flash_task();
	timeCallback++;

}
// ----- main() ---------------------------------------------------------------

void sys_clock_init();
void sys_nvic_init();

void main()
{
	//切换到外部时钟
	sys_clock_init();
	sys_nvic_init();

	timer_start();
#ifdef DEBUG
	//初始化调试RTT
	//SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#endif
	//串口打印
	uart_hal_init(USART1);
#ifdef DEBUG
	trace_printf("System clock: %u Hz\n", SystemCoreClock);
#endif

	//485模块接口
	//uart_hal_init(USART1);
	//m6311r接口
	uart_hal_init(USART3);
	//power reset m6311r
	m6311r_reset();
	sys_stop_delay(15000);
	//初始化网络接口
	gprs_connect();
	onenet_init();
	modbus_init();
	//load_cmd_para_from_flash();
	//485 50ms poll
	timerhw_create(50, 1, task_poll_485);
	timerhw_start();
//	//1s 任务调度执行
	timer1hw_create(1000, 1, task_poll_1s);
	timer1hw_start();
	//iwdg 初始化
	iwdg_init();

	while (1)
	{
		iot_onenet_task(timeCallback);
		//kick iwdg
		iwdg_kick();

		//故障状态重启
		if (reM6311Start == 1)
		{
			reM6311Start = 0;
			//关机
			m6311r_reset();
			iwdg_kick();
			//等待15s
			sys_stop_delay(5000);
			iwdg_kick();
			sys_stop_delay(5000);
			//重新开机
			m6311r_reset();
			iwdg_kick();
			sys_stop_delay(5000);
			iwdg_kick();
			gprs_connect();
			onenet_init();

		}
	}

	//test_uart5();//已测试
	//test_uart1();//已测试
	//test_uart2();//已测试

	//test_timer();//已测试
	//test_mymemery();//已测试

	//test_spi_master(); //已测试
	//test_spi_slave();
	//test_kx122();
	//test_w25q();//已测试
	//test_datasave();//已测试
	//test_save_power //已测试
	//test_save_wakeup //已测试

	while (1)
		;
}

void sys_clock_init()
{
	ErrorStatus HSEStartUpStatus;
#ifndef USE_STM32L151

	/*将外设RCC寄存器重设为缺省值*/
	RCC_DeInit();
	/*设置外部高速晶振（HSE*/
	RCC_HSEConfig(RCC_HSE_ON);
	//RCC_HSE_ON——HSE晶振打开(ON)
	/*等待HSE起振*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)	//SUCCESS：HSE晶振稳定且就绪
	{
		/*设置AHB时钟（HCLK）*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		//RCC_SYSCLK_Div1——AHB时钟=系统时钟
		/*设置高速AHB时钟（PCLK2）*/
		RCC_PCLK2Config(RCC_HCLK_Div1);
		//RCC_HCLK_Div1——APB2时钟=HCLK
		/*设置低速AHB时钟（PCLK1）*/
		RCC_PCLK1Config(RCC_HCLK_Div2);
		//RCC_HCLK_Div2——APB1时钟=HCLK/2
		/*设置FLASH存储器延时时钟周期数*/
		FLASH_SetLatency(FLASH_Latency_2);
		//FLASH_Latency_22延时周期
		/*选择FLASH预取指缓存的模式*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		//预取指缓存使能
		/*设置PLL时钟源及倍频系数*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		//PLL的输入时钟=HSE时钟频率；RCC_PLLMul_9——PLL输入时钟x9
		/*使能PLL*/
		RCC_PLLCmd(ENABLE);
		/*检查指定的RCC标志位(PLL准备好标志)设置与否*/
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		/*设置系统时钟（SYSCLK*/
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		//RCC_SYSCLKSource_PLLCLK——选择PLL作为系统时钟
		/*PLL返回用作系统时钟的时钟源*/
		while (RCC_GetSYSCLKSource() != 0x08)	//0x08：PLL作为系统时钟
		{

		}
	}
#elif
	uint32_t tmpreg = 0;
	/*将外设RCC寄存器重设为缺省值*/
	RCC_DeInit();
	/*设置外部高速晶振（HSE*/
	RCC_HSEConfig(RCC_HSE_ON);
	//RCC_HSE_ON——HSE晶振打开(ON)
	/*等待HSE起振*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)	//SUCCESS：HSE晶振稳定且就绪
	{
		/*设置AHB时钟（HCLK）*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		//RCC_SYSCLK_Div1——AHB时钟=系统时钟
		/*设置高速AHB时钟（PCLK2）*/
		RCC_PCLK2Config(RCC_HCLK_Div1);
		//RCC_HCLK_Div1——APB2时钟=HCLK
		/*设置低速AHB时钟（PCLK1）*/
		RCC_PCLK1Config(RCC_HCLK_Div1);
		//RCC_HCLK_Div2——APB1时钟=HCLK
		/*设置FLASH存储器延时时钟周期数*/
		FLASH_SetLatency(FLASH_Latency_2);
		//FLASH_Latency_22延时周期
		/*选择FLASH预取指缓存的模式*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		//预取指缓存使能
		/*设置PLL时钟源及倍频系数*/
		//RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_5);//注意在stm32l151 x8
		//外部晶振为8M时,sysclk = 8*8/2 = 32M ,apb1 = apb2 = 32M
		tmpreg = RCC->CFGR;
		/* Clear PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
		tmpreg &= CFGR_PLL_Mask;
		/* Set the PLL configuration bits */
		tmpreg |= 0x00010000 | 0x000C0000|0x00600000; //HSE 时钟源,PLL MUL= 8,PLLDIV[1:0]: PLL output division = 2
		/* Store the new value */
		RCC->CFGR = tmpreg;
		/*使能PLL*/
		RCC_PLLCmd(ENABLE);
		/*检查指定的RCC标志位(PLL准备好标志)设置与否*/
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		/*设置系统时钟（SYSCLK*/
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		//RCC_SYSCLKSource_PLLCLK——选择PLL作为系统时钟
		/*PLL返回用作系统时钟的时钟源*/
		while (RCC_GetSYSCLKSource() != 0x08)	//0x08：PLL作为系统时钟
		{

		}
	}
#endif

}

void sys_nvic_init()
{
	//0~15 个主优先级，全部为抢占优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}
