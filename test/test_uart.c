/*
 * test_uart.c
 *
 *  Created on: 2017年12月1日
 *      Author: hxbao
 */

#include "test.h"

void test_uart5()
{
	uart_hal_init(UART5);
	//等待pc发送数据
	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET command->%s\n", commd_buf);
#endif			//原样返回数据
			uart_send_data(UART5,commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

void test_uart1()
{
	uart_hal_init(USART1);
	//等待pc发送数据
	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET command->%s\n", commd_buf);
#endif			//原样返回数据
			uart_send_data(USART1,commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

void test_uart2()
{
	uart_hal_init(USART2);
	//等待pc发送数据
	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET command->%s\n", commd_buf);
#endif			//原样返回数据
			uart_send_data(USART2,commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

