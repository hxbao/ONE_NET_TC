/*
 * test_uart.c
 *
 *  Created on: 2017��12��1��
 *      Author: hxbao
 */

#include "test.h"

void test_uart5()
{
	uart_hal_init(UART5);
	//�ȴ�pc��������
	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET command->%s\n", commd_buf);
#endif			//ԭ����������
			uart_send_data(UART5,commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

void test_uart1()
{
	uart_hal_init(USART1);
	//�ȴ�pc��������
	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET command->%s\n", commd_buf);
#endif			//ԭ����������
			uart_send_data(USART1,commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

void test_uart2()
{
	uart_hal_init(USART2);
	//�ȴ�pc��������
	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET command->%s\n", commd_buf);
#endif			//ԭ����������
			uart_send_data(USART2,commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

