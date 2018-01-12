/*
 * uart.h
 *
 *  Created on: 2017年11月28日
 *      Author: hxbao
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include "stm32f10x.h"
#define UART_BUF_SIZE 512
#define UART3_BUF_SIZE 2048

extern uint16_t Uart3RxCount;


#define uart_printf(pdata) uart_send_data(USART1, (uint8_t*)pdata, strlen(pdata))

void uart_hal_init(USART_TypeDef* uart);
void uart_send_data(USART_TypeDef* uart, uint8_t *pdata, uint16_t len);

//轮询485接口特定命令帧
//uint16_t iot_uart485_recv(uint8_t *pBuf);
//uint8_t uart_read_data(USART_TypeDef uart,);
void uart_read(USART_TypeDef* uart_ins, uint8_t *pdata, uint8_t len);
uint8_t iot_uart_check_data(char* pcheck);
void uart_fflush(USART_TypeDef* uart_ins);

#endif /* INCLUDE_UART_H_ */
