/*
 * i2c.c
 *
 *  Created on: 2017年11月16日
 *      Author: hxbao
 */

#include "stm32f10x.h"
#include "i2c.h"
#include "timer.h"
#include "diag/trace.h"

#define kx122_i2c_dev_addr_w 0x3c
#define	kx122_i2c_dev_addr_r 0x3d



void i2c_hal_init(I2C_TypeDef* I2Cx)
{
	I2C_InitTypeDef I2c_init;
	GPIO_InitTypeDef gpio;
	//打开GPIO时钟
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

	if (I2Cx == I2C1)
	{

		RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);

		//sclk
		gpio.GPIO_Mode = GPIO_Mode_AF_OD;
		gpio.GPIO_Pin = GPIO_Pin_6;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		//配置GPIO口复用配置
		GPIO_Init(GPIOB, &gpio);		//sda
		gpio.GPIO_Mode = GPIO_Mode_AF_OD;
		gpio.GPIO_Pin = GPIO_Pin_7;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		//配置GPIO口复用配置
		GPIO_Init(GPIOB, &gpio);

	}
	else if (I2Cx == I2C2)
	{
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);

		//sclk
		gpio.GPIO_Mode = GPIO_Mode_AF_OD;
		gpio.GPIO_Pin = GPIO_Pin_10;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		//配置GPIO口复用配置
		GPIO_Init(GPIOB, &gpio);

		//sda
		gpio.GPIO_Mode = GPIO_Mode_AF_OD;
		gpio.GPIO_Pin = GPIO_Pin_11;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		//配置GPIO口复用配置
		GPIO_Init(GPIOB, &gpio);

	}
	I2c_init.I2C_ClockSpeed = 400000;
	I2c_init.I2C_Mode = I2C_Mode_I2C;
	I2c_init.I2C_OwnAddress1 = 0;
	I2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
	I2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2c_init.I2C_Ack = I2C_Ack_Enable;
	I2C_Init(I2Cx, &I2c_init);

	//I2C_SlaveAddressConfig(I2Cx, kx122_i2c_dev_addr_w);
	I2C_Cmd(I2Cx, ENABLE);
}

static uint8_t is_timeout = 0;
void i2c_timerout()
{
	is_timeout = 1;
#ifdef DEBUG
	trace_printf("i2c timeout\n");
#endif
}

uint8_t kx122_i2c_write_byte(I2C_TypeDef* I2Cx, uint8_t regAdd, uint8_t byte)
{
	timerhw_create(1000, 0, i2c_timerout);		//10ms ,1 time,
	is_timeout = 0;
	timerhw_start();

	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_Send7bitAddress(I2Cx, kx122_i2c_dev_addr_w, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_SendData(I2Cx, regAdd);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_SendData(I2Cx, byte);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_GenerateSTOP(I2Cx, ENABLE);
	timerhw_stop();
	return SUCESS;
}

uint8_t kx122_i2c_read_byte(I2C_TypeDef* I2Cx, uint8_t regAdd,
		uint8_t *reRxData)
{
	timerhw_create(1000, 0, i2c_timerout);		//10ms ,1 time,
	is_timeout = 0;
	timerhw_start();
	//uint8_t retByte;
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_Send7bitAddress(I2Cx, kx122_i2c_dev_addr_w, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_SendData(I2Cx, regAdd);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_Send7bitAddress(I2Cx, kx122_i2c_dev_addr_r, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	*reRxData = I2C_ReceiveData(I2Cx);
	I2C_AcknowledgeConfig(I2Cx, DISABLE);		//NACK
	I2C_GenerateSTOP(I2Cx, ENABLE);
	timerhw_stop();
	return SUCESS;
}

uint8_t kx122_i2c_read_Mbyte(I2C_TypeDef* I2Cx, uint8_t startRegAdd,
		uint8_t* pBuf, uint16_t num)
{
	timerhw_create(1000, 0, i2c_timerout);		//10ms ,1 time,
	is_timeout = 0;
	timerhw_start();
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_Send7bitAddress(I2Cx, kx122_i2c_dev_addr_w, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_SendData(I2Cx, startRegAdd);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}
	I2C_Send7bitAddress(I2Cx, kx122_i2c_dev_addr_r, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (is_timeout)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return TIMEOUT;
		}
	}

	while (num)
	{
		if (num == 1)
		{
			I2C_AcknowledgeConfig(I2Cx, DISABLE);
			I2C_GenerateSTOP(I2Cx, ENABLE);
		}

		if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
		{
			*pBuf = I2C_ReceiveData(I2Cx);
			I2C_AcknowledgeConfig(I2Cx, ENABLE);		//NACK
			pBuf++;
			num--;
		}
	}
	//enable ack for next reciption
	I2C_AcknowledgeConfig(I2Cx, ENABLE);		//ACK
	timerhw_stop();
	return SUCESS;
}
