/*
 * modbus.c
 *
 *  Created on: 2018��1��9��
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

//50ms ��ѯ����,�ж���ִ��,���в��е������жϵĺ���
void task_poll_485()
{

	modbus_task();
}

//1s ��ѯ����

void task_poll_1s()
{

#ifdef DEBUG
	//trace_printf("call task poll 1s\n");
#endif
	//poll_store_commd();
	//cmds��flash����ͬ��
	//store_cmd_para_to_flash_task();
	timeCallback++;

}
// ----- main() ---------------------------------------------------------------

void sys_clock_init();
void sys_nvic_init();

void main()
{
	//�л����ⲿʱ��
	sys_clock_init();
	sys_nvic_init();

	timer_start();
#ifdef DEBUG
	//��ʼ������RTT
	//SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#endif
	//���ڴ�ӡ
	uart_hal_init(USART1);
#ifdef DEBUG
	trace_printf("System clock: %u Hz\n", SystemCoreClock);
#endif

	//485ģ��ӿ�
	//uart_hal_init(USART1);
	//m6311r�ӿ�
	uart_hal_init(USART3);
	//power reset m6311r
	m6311r_reset();
	sys_stop_delay(15000);
	//��ʼ������ӿ�
	gprs_connect();
	onenet_init();
	modbus_init();
	//load_cmd_para_from_flash();
	//485 50ms poll
	timerhw_create(50, 1, task_poll_485);
	timerhw_start();
//	//1s �������ִ��
	timer1hw_create(1000, 1, task_poll_1s);
	timer1hw_start();
	//iwdg ��ʼ��
	iwdg_init();

	while (1)
	{
		iot_onenet_task(timeCallback);
		//kick iwdg
		iwdg_kick();

		//����״̬����
		if (reM6311Start == 1)
		{
			reM6311Start = 0;
			//�ػ�
			m6311r_reset();
			iwdg_kick();
			//�ȴ�15s
			sys_stop_delay(5000);
			iwdg_kick();
			sys_stop_delay(5000);
			//���¿���
			m6311r_reset();
			iwdg_kick();
			sys_stop_delay(5000);
			iwdg_kick();
			gprs_connect();
			onenet_init();

		}
	}

	//test_uart5();//�Ѳ���
	//test_uart1();//�Ѳ���
	//test_uart2();//�Ѳ���

	//test_timer();//�Ѳ���
	//test_mymemery();//�Ѳ���

	//test_spi_master(); //�Ѳ���
	//test_spi_slave();
	//test_kx122();
	//test_w25q();//�Ѳ���
	//test_datasave();//�Ѳ���
	//test_save_power //�Ѳ���
	//test_save_wakeup //�Ѳ���

	while (1)
		;
}

void sys_clock_init()
{
	ErrorStatus HSEStartUpStatus;
#ifndef USE_STM32L151

	/*������RCC�Ĵ�������Ϊȱʡֵ*/
	RCC_DeInit();
	/*�����ⲿ���پ���HSE*/
	RCC_HSEConfig(RCC_HSE_ON);
	//RCC_HSE_ON����HSE�����(ON)
	/*�ȴ�HSE����*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)	//SUCCESS��HSE�����ȶ��Ҿ���
	{
		/*����AHBʱ�ӣ�HCLK��*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		//RCC_SYSCLK_Div1����AHBʱ��=ϵͳʱ��
		/*���ø���AHBʱ�ӣ�PCLK2��*/
		RCC_PCLK2Config(RCC_HCLK_Div1);
		//RCC_HCLK_Div1����APB2ʱ��=HCLK
		/*���õ���AHBʱ�ӣ�PCLK1��*/
		RCC_PCLK1Config(RCC_HCLK_Div2);
		//RCC_HCLK_Div2����APB1ʱ��=HCLK/2
		/*����FLASH�洢����ʱʱ��������*/
		FLASH_SetLatency(FLASH_Latency_2);
		//FLASH_Latency_22��ʱ����
		/*ѡ��FLASHԤȡָ�����ģʽ*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		//Ԥȡָ����ʹ��
		/*����PLLʱ��Դ����Ƶϵ��*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		//PLL������ʱ��=HSEʱ��Ƶ�ʣ�RCC_PLLMul_9����PLL����ʱ��x9
		/*ʹ��PLL*/
		RCC_PLLCmd(ENABLE);
		/*���ָ����RCC��־λ(PLL׼���ñ�־)�������*/
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		/*����ϵͳʱ�ӣ�SYSCLK*/
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		//RCC_SYSCLKSource_PLLCLK����ѡ��PLL��Ϊϵͳʱ��
		/*PLL��������ϵͳʱ�ӵ�ʱ��Դ*/
		while (RCC_GetSYSCLKSource() != 0x08)	//0x08��PLL��Ϊϵͳʱ��
		{

		}
	}
#elif
	uint32_t tmpreg = 0;
	/*������RCC�Ĵ�������Ϊȱʡֵ*/
	RCC_DeInit();
	/*�����ⲿ���پ���HSE*/
	RCC_HSEConfig(RCC_HSE_ON);
	//RCC_HSE_ON����HSE�����(ON)
	/*�ȴ�HSE����*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)	//SUCCESS��HSE�����ȶ��Ҿ���
	{
		/*����AHBʱ�ӣ�HCLK��*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		//RCC_SYSCLK_Div1����AHBʱ��=ϵͳʱ��
		/*���ø���AHBʱ�ӣ�PCLK2��*/
		RCC_PCLK2Config(RCC_HCLK_Div1);
		//RCC_HCLK_Div1����APB2ʱ��=HCLK
		/*���õ���AHBʱ�ӣ�PCLK1��*/
		RCC_PCLK1Config(RCC_HCLK_Div1);
		//RCC_HCLK_Div2����APB1ʱ��=HCLK
		/*����FLASH�洢����ʱʱ��������*/
		FLASH_SetLatency(FLASH_Latency_2);
		//FLASH_Latency_22��ʱ����
		/*ѡ��FLASHԤȡָ�����ģʽ*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		//Ԥȡָ����ʹ��
		/*����PLLʱ��Դ����Ƶϵ��*/
		//RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_5);//ע����stm32l151 x8
		//�ⲿ����Ϊ8Mʱ,sysclk = 8*8/2 = 32M ,apb1 = apb2 = 32M
		tmpreg = RCC->CFGR;
		/* Clear PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
		tmpreg &= CFGR_PLL_Mask;
		/* Set the PLL configuration bits */
		tmpreg |= 0x00010000 | 0x000C0000|0x00600000; //HSE ʱ��Դ,PLL MUL= 8,PLLDIV[1:0]: PLL output division = 2
		/* Store the new value */
		RCC->CFGR = tmpreg;
		/*ʹ��PLL*/
		RCC_PLLCmd(ENABLE);
		/*���ָ����RCC��־λ(PLL׼���ñ�־)�������*/
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		/*����ϵͳʱ�ӣ�SYSCLK*/
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		//RCC_SYSCLKSource_PLLCLK����ѡ��PLL��Ϊϵͳʱ��
		/*PLL��������ϵͳʱ�ӵ�ʱ��Դ*/
		while (RCC_GetSYSCLKSource() != 0x08)	//0x08��PLL��Ϊϵͳʱ��
		{

		}
	}
#endif

}

void sys_nvic_init()
{
	//0~15 �������ȼ���ȫ��Ϊ��ռ���ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}
