#include "stm32f10x.h"
#include "g_conf.h"
#include "commd_if.h"
#include "string.h"
#ifdef DEBUG
#include "diag/Trace.h"
#endif

#define SPI_SLAVE_DATA_BUF_SIZE 512

uint16_t spi_slave_recv_buf[SPI_SLAVE_DATA_BUF_SIZE];
//发送缓冲数据
uint16_t spi_slave_tx_buf[SPI_SLAVE_DATA_BUF_SIZE];

void spi_slave_hal_init(void)
{
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi;
	DMA_InitTypeDef dma; //DMA初始化结构体
	//NVIC_InitTypeDef nvic; //NVIC初始化结构体

	//1、时钟打开初始化
	//RCC_AHBPeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//2、gpio 管脚初始化
	//PA2作为通知SPI master的中断信号
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);//默认为低电平

	//MISO
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_6;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	//SCLK,NCS,MOSI
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_7| GPIO_Pin_5 ;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);


	//3、spi 模块配置
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	spi.SPI_CPHA = SPI_CPHA_2Edge;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CRCPolynomial = 7;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_Mode = SPI_Mode_Slave;
	spi.SPI_NSS = SPI_NSS_Hard;

	SPI_Init(SPI1, &spi);

	//4、dma配置 ,spi_rx channel
	DMA_DeInit(DMA1_Channel2);
	dma.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
	dma.DMA_MemoryBaseAddr = (uint32_t) spi_slave_recv_buf;
	dma.DMA_DIR = DMA_DIR_PeripheralSRC;
	dma.DMA_BufferSize = SPI_SLAVE_DATA_BUF_SIZE;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_Priority = DMA_Priority_VeryHigh;
	dma.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &dma);
	//DMA传输完成中断
	//DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);//不使用DMA传输完成中断

	//spi dma tx 发送
	//使能spi的dma 发送和接收功能
	//SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
	SPI_Cmd(SPI1, ENABLE);
	DMA_Cmd(DMA1_Channel2, ENABLE);

}

//use spi1 pa4,pa5,pa6,pa7
void spi1_master_hal_init(void)
{
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi;
	//DMA_InitTypeDef dma; //DMA初始化结构体
	//NVIC_InitTypeDef nvic; //NVIC初始化结构体

	//1、时钟打开初始化
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//2、gpio 管脚初始化

	//CS
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_4;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	//SCLK,MOSI
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	//MISO
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Pin = GPIO_Pin_6 ;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	//3、spi 模块配置
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	spi.SPI_CPHA = SPI_CPHA_1Edge;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CRCPolynomial = 7;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_NSS = SPI_NSS_Soft;

	SPI_Init(SPI1, &spi);
	SPI_Cmd(SPI1, ENABLE);


}

uint8_t spi1_master_read_bytes(uint8_t *buff, int size)
{
	int i;
	if(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == SET)
		i = SPI_I2S_ReceiveData(SPI1);

	for(i = 0; i < size; i++)
	{
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI1, 0xff);
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
		buff[i] = SPI_I2S_ReceiveData(SPI1);
	}
	return 0;
}


uint8_t spi1_master_write_bytes(uint8_t *buff, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		while(!(SPI1->SR & SPI_I2S_FLAG_TXE));
		SPI1->DR = buff[i];
	}
	while(!(SPI1->SR & SPI_I2S_FLAG_TXE));
	while(SPI1->SR & SPI_I2S_FLAG_BSY);

	return 0;
}

//use spi2 pb12,pb13,pb14,pb15
void spi_master_hal_init(void)
{
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi;
	//DMA_InitTypeDef dma; //DMA初始化结构体
	//NVIC_InitTypeDef nvic; //NVIC初始化结构体

	//1、时钟打开初始化
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//2、gpio 管脚初始化

	//SCLK,MOSI
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);

	//SCLK,MOSI
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);

	//MISO
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Pin = GPIO_Pin_14 ;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);

	//3、spi 模块配置
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	spi.SPI_CPHA = SPI_CPHA_1Edge;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CRCPolynomial = 7;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_NSS = SPI_NSS_Soft;

	SPI_Init(SPI2, &spi);
	SPI_Cmd(SPI2, ENABLE);


}

uint8_t spi_master_read_bytes(uint8_t *buff, int size)
{
	int i;
	if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		i = SPI_I2S_ReceiveData(SPI2);

	for(i = 0; i < size; i++)
	{
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI2, 0xff);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		buff[i] = SPI_I2S_ReceiveData(SPI2);
	}
	return 0;
}


uint8_t spi_master_write_bytes(uint8_t *buff, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		while(!(SPI2->SR & SPI_I2S_FLAG_TXE));
		SPI2->DR = buff[i];
	}
	while(!(SPI2->SR & SPI_I2S_FLAG_TXE));
	while(SPI2->SR & SPI_I2S_FLAG_BSY);

	return 0;
}

//使用一根IO控制线作为外部中断，监控接收spi的一帧完整数据完成信号
//数据发送完成后，发送一个脉冲信号后，重新拉成低电平
//PA3作为外部输入中断
void spi_recv_finish_exit_config()
{
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	//将GPIO管脚与外部中断线连接
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	nvic.NVIC_IRQChannel = EXTI3_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = INTER_EXTL3;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
}

//外部spi master 查询到pt_rx io 信号拉高之后，可以通过spi读取一帧数据
//数据帧的长度固定为512字节
//数据的解析，根据应用层协议解析
//如果前次发送的数据还没有被读取，返回0，发送失败，否则发送成功
uint8_t spi_slave_tx_data(uint8_t *pdata, uint8_t len)
{
	uint16_t i;
	//查看spi 发送数据状态pin，树莓派IO拉低IO
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == RESET)
	{
		//拉低pt_tx io 信号，通知spi master 可以读取数据
		//SET_GPIO_PA2_HIGH();
		GPIO_SetBits(GPIOA, GPIO_Pin_2);

		for(i= 0;i<len;i++)
		{
			while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=RESET)
				;
			SPI_I2S_SendData(SPI1,*(pdata+i));
		}
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);

		return 1;
	}
	else
	{
		return 0;
	}

}

//外部中断处理
void EXTI3_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line3); //清除标志
#ifdef DEBUG
		trace_printf("DEBUG INFO:get spi slave finish interrupt\n");
#endif
		DMA_Cmd(DMA1_Channel2, DISABLE);
		//得到spi接收的dma接收的长度
		commd_cur_len = SPI_SLAVE_DATA_BUF_SIZE - DMA1_Channel2->CNDTR;
		memcpy(commd_buf, spi_slave_recv_buf, commd_cur_len);
		//重新装载数据长度，重新启动传输
		DMA1_Channel2->CNDTR = SPI_SLAVE_DATA_BUF_SIZE;
		DMA_Cmd(DMA1_Channel2, ENABLE);
	}
}

