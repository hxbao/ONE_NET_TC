/*
 * uart.c
 *
 *  Created on: 2017��11��28��
 *      Author: hxbao
 */
#include "stm32f10x.h"
#include "g_conf.h"
#include "uart.h"
#include "commd_if.h"
#include "string.h"
#include "diag/trace.h"
#include "timer.h"
#include "SEGGER_RTT.h"

#define UART_TX_BUF_SIZE 1024
uint8_t UART_TX_BUF[UART_TX_BUF_SIZE];

//����buffer
uint8_t UART1_BUF[UART_BUF_SIZE];
uint8_t UART2_BUF[128];
uint8_t UART3_BUF[UART3_BUF_SIZE];
//����buffer�Ķ�����λ��ָ��
uint8_t UART1_BUF_Pos = 0;
uint8_t UART2_BUF_Pos = 0;

uint16_t Uart1RxCount = 0;
uint16_t Uart3RxCount = 0;

void uart_hal_init(USART_TypeDef* uart_ins)
{
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart_init;
	DMA_InitTypeDef dma; //DMA��ʼ���ṹ��
	NVIC_InitTypeDef nvic; //NVIC��ʼ���ṹ��

	if (uart_ins == USART1) //485�ӿ�
	{
		//1��ʱ�Ӵ򿪳�ʼ��
		RCC_APB2PeriphClockCmd(
				RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
						| RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		//PA9->TX
		gpio.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio.GPIO_Pin = GPIO_Pin_9;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio);

		//PA9->RX
		gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpio.GPIO_Pin = GPIO_Pin_10;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio);

		//RE
		gpio.GPIO_Mode = GPIO_Mode_Out_PP;
		gpio.GPIO_Pin = GPIO_Pin_8;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &gpio);
		//ֻ����
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);

		usart_init.USART_BaudRate = 9600;
		usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		usart_init.USART_WordLength = USART_WordLength_8b;
		usart_init.USART_StopBits = USART_StopBits_1;
		usart_init.USART_Parity = USART_Parity_No;
		USART_Init(uart_ins, &usart_init);
		//dmar rx
//		DMA_DeInit(DMA1_Channel5);
//		dma.DMA_PeripheralBaseAddr = (uint32_t) &(uart_ins->DR);
//		dma.DMA_MemoryBaseAddr = (uint32_t) UART1_BUF;
//		dma.DMA_DIR = DMA_DIR_PeripheralSRC;
//		dma.DMA_BufferSize = UART_BUF_SIZE;
//		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
//		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//		dma.DMA_Mode = DMA_Mode_Circular;
//		dma.DMA_Priority = DMA_Priority_Medium;
//		dma.DMA_M2M = DMA_M2M_Disable;
//		DMA_Init(DMA1_Channel5, &dma);
//		DMA_Cmd(DMA1_Channel5, ENABLE);
//
//		USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

		nvic.NVIC_IRQChannel = USART1_IRQn;
		nvic.NVIC_IRQChannelPreemptionPriority = INTER_UART1;
		nvic.NVIC_IRQChannelSubPriority = 0;
		nvic.NVIC_IRQChannelCmd = ENABLE;
		USART_ITConfig(uart_ins, USART_IT_RXNE, ENABLE);
		NVIC_Init(&nvic);
	}
	else if (uart_ins == USART2)
	{
		//1��ʱ�Ӵ򿪳�ʼ��
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		RCC_APB2PeriphClockCmd(
		RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		//2��gpio �ܽų�ʼ��
		//PA2->TX
		gpio.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio.GPIO_Pin = GPIO_Pin_2;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio);

		//PA3->RX
		gpio.GPIO_Mode = GPIO_Mode_IPU;
		gpio.GPIO_Pin = GPIO_Pin_3;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio);

		usart_init.USART_BaudRate = 9600;		//115200;//921600;
		usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		usart_init.USART_WordLength = USART_WordLength_8b;
		usart_init.USART_StopBits = USART_StopBits_1;
		usart_init.USART_Parity = USART_Parity_No;
		USART_Init(uart_ins, &usart_init);
		//uart ����dma ͨ��
		DMA_DeInit(DMA1_Channel6);
		dma.DMA_PeripheralBaseAddr = (uint32_t) &(uart_ins->DR);
		;
		dma.DMA_MemoryBaseAddr = (uint32_t) UART2_BUF;
		dma.DMA_DIR = DMA_DIR_PeripheralSRC;
		dma.DMA_BufferSize = UART_BUF_SIZE;
		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		dma.DMA_Mode = DMA_Mode_Circular;
		dma.DMA_Priority = DMA_Priority_Medium;
		dma.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(DMA1_Channel6, &dma);

		DMA_Cmd(DMA1_Channel6, ENABLE);
		USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

//		//uart ����dma ͨ��
//		DMA_DeInit(DMA1_Channel7);
//		dma.DMA_PeripheralBaseAddr = (uint32_t)&(uart_ins->DR);;
//		dma.DMA_MemoryBaseAddr = (uint32_t) UART_TX_BUF;
//		dma.DMA_DIR = DMA_DIR_PeripheralDST;
//		dma.DMA_BufferSize = UART_BUF_SIZE;
//		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
//		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//		dma.DMA_Mode = DMA_Mode_Normal;
//		dma.DMA_Priority = DMA_Priority_Medium;
//		dma.DMA_M2M = DMA_M2M_Disable;
//		DMA_Init(DMA1_Channel7, &dma);
//		DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);
//		//
//		//DMA_Cmd(DMA1_Channel7, ENABLE);
//		USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);

		//ʹ��uart��idle
		nvic.NVIC_IRQChannel = USART2_IRQn;
		nvic.NVIC_IRQChannelPreemptionPriority = INTER_UART2;
		nvic.NVIC_IRQChannelSubPriority = 0;
		nvic.NVIC_IRQChannelCmd = ENABLE;

		USART_ITConfig(uart_ins, USART_IT_IDLE, ENABLE);
		NVIC_Init(&nvic);

	}
	else if (uart_ins == USART3)
	{
		//1��ʱ�Ӵ򿪳�ʼ��
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,
				ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
		//2��gpio �ܽų�ʼ��
		//PC12->TX
		gpio.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio.GPIO_Pin = GPIO_Pin_10;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &gpio);

		//PD2->RX
		gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpio.GPIO_Pin = GPIO_Pin_11;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &gpio);

		usart_init.USART_BaudRate = 115200;
		usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		usart_init.USART_WordLength = USART_WordLength_8b;
		usart_init.USART_StopBits = USART_StopBits_1;
		usart_init.USART_Parity = USART_Parity_No;
		USART_Init(uart_ins, &usart_init);

//		DMA_DeInit(DMA2_Channel3);
//		dma.DMA_PeripheralBaseAddr = uart_ins->DR;
//		dma.DMA_MemoryBaseAddr = (uint32_t) UART3_BUF;
//		dma.DMA_DIR = DMA_DIR_PeripheralSRC;
//		dma.DMA_BufferSize = UART_BUF_SIZE;
//		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
//		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//		dma.DMA_Mode = DMA_Mode_Normal;
//		dma.DMA_Priority = DMA_Priority_Medium;
//		dma.DMA_M2M = DMA_M2M_Disable;
//		DMA_Init(DMA2_Channel3, &dma);
//		DMA_Cmd(DMA2_Channel3, ENABLE);

		nvic.NVIC_IRQChannel = USART3_IRQn;
		nvic.NVIC_IRQChannelPreemptionPriority = INTER_UART5;
		nvic.NVIC_IRQChannelSubPriority = 0;
		nvic.NVIC_IRQChannelCmd = ENABLE;

		//��Ӧuart��idle�ж�
		//USART_ITConfig(uart_ins, USART_IT_IDLE, ENABLE);
		//ʹ�ܽ��շǿ��ж�
		USART_ITConfig(uart_ins, USART_IT_RXNE, ENABLE);
		NVIC_Init(&nvic);
	}

	USART_Cmd(uart_ins, ENABLE);
}

//��������
void uart_send_data(USART_TypeDef* uart_ins, uint8_t *pdata, uint16_t len)
{
	uint16_t i = 0;
	if (uart_ins == USART1)
	{

		GPIO_SetBits(GPIOB, GPIO_Pin_8);
		for (i = 0; i < len; i++)
		{
			while (USART_GetFlagStatus(uart_ins, USART_FLAG_TXE) == RESET)
				;
			USART_SendData(uart_ins, *(pdata + i));
		}
		//while (USART_GetFlagStatus(uart_ins, USART_FLAG_TXE) == RESET);
		while (USART_GetFlagStatus(uart_ins, USART_FLAG_TC) == RESET)
			;

		GPIO_ResetBits(GPIOB, GPIO_Pin_8);

	}
	else
	{
		for (i = 0; i < len; i++)
		{
			while (USART_GetFlagStatus(uart_ins, USART_FLAG_TXE) == RESET)
				;
			USART_SendData(uart_ins, *(pdata + i));
		}
		while (USART_GetFlagStatus(uart_ins, USART_FLAG_TC) == RESET)
			;
	}

}

//usart2 ����dma ��������ж�
void DMA1_Channel7_IRQHandler()
{
	DMA_ClearITPendingBit(DMA1_IT_TC7);		//(DMA1_FLAG_TC7);
	//�ر�DMA
	DMA_Cmd(DMA1_Channel7, DISABLE);
}

////�жϽ����ݴ����ݿ�
//uint8_t uart1_temp_buf[64];
//uint8_t uart1_temp_buf_count = 0;
////�����жϴ���
void USART1_IRQHandler()
{
uint8_t i;
	if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART1); // Clear IDLE interrupt flag bit
#ifdef DEBUG
		//trace_printf("receive uart3 data:%s\n", UART3_BUF);
#endif
		//Uart3RxCount = 0;
	}
	else if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		//�����Ѿ���������buffer��
		if (Uart1RxCount == UART_BUF_SIZE)
		{
			Uart1RxCount = 0;
		}

		UART1_BUF[Uart1RxCount] = USART_ReceiveData(USART1);

		if(UART1_BUF[0] != 0x5a)
		{
			Uart1RxCount =0;
			return ;
		}else
		{
			Uart1RxCount++;
		}
		//����3���ֽ��ж�
		if(Uart1RxCount>=3)
		{
#ifdef DEBUG
				//SEGGER_RTT_printf(0,"0x%x 0x%x 0x%x\n",UART1_BUF[0],UART1_BUF[1],UART1_BUF[2]);
#endif
			if((UART1_BUF[0] == 0x5a) && (UART1_BUF[1] == 0xa5) && (UART1_BUF[2] == 0x0a))
			{//���ܱ�������

#ifdef DEBUG
				if(UART1_BUF[3] > 4)
				{
					//SEGGER_RTT_printf(0,"len %d\n",UART1_BUF[3]);
				}
#endif
				if(Uart1RxCount == UART1_BUF[3]+2)
				{
					for(i=0;i<Uart1RxCount;i++)
					{
						modbus_temp_buf[i] = UART1_BUF[i];
					}
					modbus_temp_len = Uart1RxCount;
					Uart1RxCount = 0;
					UART1_BUF[0] =0;
				}

			}else
			{
				Uart1RxCount = 0;
			}
		}


		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void USART2_IRQHandler()
{
	if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART2); // Clear IDLE interrupt flag bit

		DMA_Cmd(DMA1_Channel6, DISABLE);

		commd_cur_len = UART_BUF_SIZE - DMA1_Channel6->CNDTR;
		memcpy(commd_buf, UART2_BUF, commd_cur_len);
		commd_buf[commd_cur_len] = 0;
		//����װ�����ݳ��ȣ�������������
		DMA1_Channel6->CNDTR = UART_BUF_SIZE;
		DMA_Cmd(DMA1_Channel6, ENABLE);
	}
}

//at �����,������dma�������ݵķ�ʽ
void USART3_IRQHandler()
{

	if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART3); // Clear IDLE interrupt flag bit
#ifdef DEBUG
		//trace_printf("receive uart3 data:%s\n", UART3_BUF);
#endif
		//Uart3RxCount = 0;
	}
	else if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		//�����Ѿ���������buffer��
		if (Uart3RxCount == UART3_BUF_SIZE)
		{
			Uart3RxCount = 0;
		}
		UART3_BUF[Uart3RxCount++] = USART_ReceiveData(USART3);
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}

void uart_fflush(USART_TypeDef* uart_ins)
{
	if (uart_ins == USART3)
	{
		memset(UART3_BUF, 0, Uart3RxCount);
		Uart3RxCount = 0;
	}
	else if (uart_ins == USART1)
	{

	}
	else if (uart_ins == USART2)
	{

	}
}

//��uartx buffer�ж�ȡlen���ֽڵ�����
void uart_read(USART_TypeDef* uart_ins, uint8_t *pdata, uint8_t len)
{
	if (uart_ins == USART3)
	{
		memcpy(pdata, UART3_BUF, len);
		//*(pdata + Uart3RxCount) = 0;
	}
	else if (uart_ins == USART1)
	{

	}
	else if (uart_ins == USART2)
	{

	}
}

//uint8_t iot_uart_check_data(char* pcheck)
//{
//"+CIOTCMD:"
//uint16_t i;

//	if (strstr(UART3_BUF, pcheck) != NULL)
//	{
//
//		return 1;
//	}
//	else
//		return 0;

//}
////��ѯ485�ӿ��ض�����֡
//uint16_t iot_uart485_recv(uint8_t *pBuf)
//{
//	uint16_t frameLen;
//	uint8_t* point5a = 0;
//	uint16_t i;
//
//	//DMA5 ͨ���Ѿ������˶����ֽڵ�����
//	//frameLen = UART_BUF_SIZE - (DMA1_Channel5->CNDTR);
//	frameLen = Uart1RxCount;
//	if (frameLen)
//	{
//		//��Ѱһ֡����
//		for (i = 0; i < UART_BUF_SIZE - 4; i++)
//		{
//			if (UART1_BUF[i] == 0x5a && UART1_BUF[i + 1] == 0xa5)
//			{
//				if (UART1_BUF[i + 2] == 0x0a)
//				{
//					//������ݳ���
//					uint16_t len;
//					len = UART1_BUF[i + 3];
//
//					//��������֡û��ѭ��
//					if (frameLen > len)
//					{
//						uint8_t sum = 0;
//						uint8_t j;
//						for (j = 0; j < len + 1; j++)
//						{
//							sum += UART1_BUF[i + j];
//						}
//						if (sum == UART1_BUF[i + j])
//						{ //У��ͨ��
//							point5a = &UART1_BUF[i];
//							memcpy(pBuf, point5a, len + 2);
//							memset(UART1_BUF, 0, UART_BUF_SIZE);
//							return len + 2;
//						}
//					}
//					else //��ѭ��
//					{
//						uint8_t sum = 0;
//						uint8_t j;
//						uint8_t count = 0;
//						uint8_t pos = 0;
//						for (j = 0; j < len + 1; j++)
//						{
//							if(i+j>UART_BUF_SIZE)
//							{
//								count = (i+j - UART_BUF_SIZE);
//
//							}else
//								if(i+j==UART_BUF_SIZE)
//								{
//									count = i+j;
//									pos = j;
//								}
//							else
//							{
//								count = i+j;
//							}
//
//							sum += UART1_BUF[count];
//						}
//						if (sum == UART1_BUF[count])
//						{ //У��ͨ��
//							point5a = &UART1_BUF[i];
//							//���ο���
//							memcpy(pBuf, point5a, pos);
//							memcpy(pBuf+pos, 0, len + 2 - pos);
//
//							memset(UART1_BUF, 0, UART_BUF_SIZE);
//							return len + 2;
//						}
//					}
//
//				}
//				else // 0x0b ����֡������������
//				{
//
//				}
//			}
//		}
//
//		return 0;
//	}
//	else
//	{
//		return 0;
//	}
//}
