#include "stm32f10x.h"
#include "g_conf.h"
#include "commd_if.h"
#include "string.h"
#ifdef DEBUG
#include "diag/Trace.h"
#endif

#define SPI_SLAVE_DATA_BUF_SIZE 512

uint16_t spi_slave_recv_buf[SPI_SLAVE_DATA_BUF_SIZE];
//���ͻ�������
uint16_t spi_slave_tx_buf[SPI_SLAVE_DATA_BUF_SIZE];

void spi_slave_hal_init(void)
{
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi;
	DMA_InitTypeDef dma; //DMA��ʼ���ṹ��
	//NVIC_InitTypeDef nvic; //NVIC��ʼ���ṹ��

	//1��ʱ�Ӵ򿪳�ʼ��
	//RCC_AHBPeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//2��gpio �ܽų�ʼ��
	//PA2��Ϊ֪ͨSPI master���ж��ź�
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);//Ĭ��Ϊ�͵�ƽ

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


	//3��spi ģ������
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

	//4��dma���� ,spi_rx channel
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
	//DMA��������ж�
	//DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);//��ʹ��DMA��������ж�

	//spi dma tx ����
	//ʹ��spi��dma ���ͺͽ��չ���
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
	//DMA_InitTypeDef dma; //DMA��ʼ���ṹ��
	//NVIC_InitTypeDef nvic; //NVIC��ʼ���ṹ��

	//1��ʱ�Ӵ򿪳�ʼ��
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//2��gpio �ܽų�ʼ��

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

	//3��spi ģ������
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
	//DMA_InitTypeDef dma; //DMA��ʼ���ṹ��
	//NVIC_InitTypeDef nvic; //NVIC��ʼ���ṹ��

	//1��ʱ�Ӵ򿪳�ʼ��
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//2��gpio �ܽų�ʼ��

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

	//3��spi ģ������
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

//ʹ��һ��IO��������Ϊ�ⲿ�жϣ���ؽ���spi��һ֡������������ź�
//���ݷ�����ɺ󣬷���һ�������źź��������ɵ͵�ƽ
//PA3��Ϊ�ⲿ�����ж�
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
	//��GPIO�ܽ����ⲿ�ж�������
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

//�ⲿspi master ��ѯ��pt_rx io �ź�����֮�󣬿���ͨ��spi��ȡһ֡����
//����֡�ĳ��ȹ̶�Ϊ512�ֽ�
//���ݵĽ���������Ӧ�ò�Э�����
//���ǰ�η��͵����ݻ�û�б���ȡ������0������ʧ�ܣ������ͳɹ�
uint8_t spi_slave_tx_data(uint8_t *pdata, uint8_t len)
{
	uint16_t i;
	//�鿴spi ��������״̬pin����ݮ��IO����IO
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == RESET)
	{
		//����pt_tx io �źţ�֪ͨspi master ���Զ�ȡ����
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

//�ⲿ�жϴ���
void EXTI3_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line3); //�����־
#ifdef DEBUG
		trace_printf("DEBUG INFO:get spi slave finish interrupt\n");
#endif
		DMA_Cmd(DMA1_Channel2, DISABLE);
		//�õ�spi���յ�dma���յĳ���
		commd_cur_len = SPI_SLAVE_DATA_BUF_SIZE - DMA1_Channel2->CNDTR;
		memcpy(commd_buf, spi_slave_recv_buf, commd_cur_len);
		//����װ�����ݳ��ȣ�������������
		DMA1_Channel2->CNDTR = SPI_SLAVE_DATA_BUF_SIZE;
		DMA_Cmd(DMA1_Channel2, ENABLE);
	}
}

