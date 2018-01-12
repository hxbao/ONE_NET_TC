/*
 * modbus.c
 *
 *  Created on: 2018年1月9日
 *      Author: hxbao
 */

#include "uart.h"
#include "stdint.h"
#include "string.h"
#include "m6311r.h"
#include "modbus.h"
#include "SEGGER_RTT.h"


#define UART_485 USART1

#define iot_uart485_send(pbuf,len) uart_send_data(UART_485, pbuf, len)

// //485数据暂存缓冲区，循环缓冲区
uint8_t modbus_temp_buf[TEMP_BUF_SIZE];
uint8_t modbus_temp_len = 0;

//从485看过来的两个缓冲区，读缓冲区和写缓冲区
uint8_t modbus_readBuf[256] =
{ 0 };
uint8_t modbus_writeBuf[256] =
{ 0 };

//modbus 读写缓冲区的计数器，即是标识数据，当计数为0时，表示没有新的数据帧
uint8_t readBufCnt = 0;
uint8_t writeBufCnt = 0;

uint8_t lasttimeSendData2OnenetStu = 0;
uint8_t stu_count = 0;

/* ----------------------- Static variables ---------------------------------*/
#define MB_DEVICE_ID 0x0a

uint8_t mbDeviceId = MB_DEVICE_ID;
uint8_t IsWrite485Buffer = 0;

int8_t modbus_check(uint8_t *pDataBuf, uint16_t length)
{
	uint8_t recvAddr = *(pDataBuf + 2);
	uint8_t i;
	uint8_t sum = 0;

	if (recvAddr == mbDeviceId)
	{
#ifdef _USE_CRC_VERIFY
		if(CrcCalculate(pDataBuf, length,0xffff) == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
#else //累加和校验
		//ZegmaCalculate(pDataBuf, length);
		//校验和校验
		for (i = 0; i < length - 1; i++)
		{
			sum += *(pDataBuf + i);
		}

		if (sum == *(pDataBuf + length - 1))
		{
			return 0;
		}
		else
		{
			return -1;
		}

#endif

	}
	else
	{
		return -1;
	}
}

//数据帧的头部已经包含识别码
void dispatch_modbus(uint8_t *modbus_data)
{
	uint8_t i;
	uint8_t sum = 0;
	uint8_t funcode;
	uint8_t mb_len;        //modbus帧的数据长度
	uint8_t valid_data_len;        //有效数据长度

	mb_len = modbus_data[1 + 2];
	funcode = modbus_data[2 + 2];
	valid_data_len = mb_len - 4;

	switch (funcode)
	{
	case 0x01:        //发送有效数据区的数据
		memcpy(modbus_writeBuf, &modbus_data[3 + 2], valid_data_len);
		//标识有新的485数据帧需要上传；
		writeBufCnt = valid_data_len;
		modbus_data[2 + 1] = 0x05;
		//修改返回帧的功能码
		modbus_data[2 + 2] = 0x03;
		//修改返回帧发送状态
		if (lasttimeSendData2OnenetStu)
			modbus_data[2 + 3] = 0x02;
		else
			modbus_data[2 + 3] = 0x01;
		//修改校验和
		for (i = 0; i < 6; i++)
		{
			sum += modbus_data[i];
		}
		modbus_data[6] = sum;
		//返回数据帧
		iot_uart485_send(modbus_data, 7);
		//发送有效数据到onenet
		IsWrite485Buffer = valid_data_len; //设置标志，在onenet任务里发送
		break;
	case 0x02:			//读取缓冲区的数据
		if (readBufCnt)
		{
			//修改长度码
			modbus_data[2 + 1] = readBufCnt + 4;
			//修改返回帧的功能码
			modbus_data[2 + 2] = 0x05;
			//填充返回数据
			memcpy(&modbus_data[5], modbus_readBuf, readBufCnt);
			//修改校验和
			for (i = 0; i < readBufCnt + 6 - 1; i++)
			{
				sum += modbus_data[i];
			}
			modbus_data[i] = sum;
			mb_len = readBufCnt + 6;
			readBufCnt = 0; //清空readBufCnt
		}
		else
		{
			//修改长度码
			modbus_data[2 + 1] = 0x06;
			//修改返回帧的功能码
			modbus_data[2 + 2] = 0x04;

			//修改返回帧发送状态

			//else
			if (stu_count == 1)
			{
				if (lasttimeSendData2OnenetStu)
					modbus_data[2 + 3] = 0x02; //发送失败
				else
					modbus_data[2 + 3] = 0x01; //发送成功
				stu_count = 0;
			}
			else
				modbus_data[2 + 3] = 0x00;

			modbus_data[2 + 4] = one_connect_state; //onenet  通信连接状态

			//修改校验和
			for (i = 0; i < 7; i++)
			{
				sum += modbus_data[i];
			}
			modbus_data[i] = sum;
			mb_len = 8;
		}
		//返回数据帧
		iot_uart485_send(modbus_data, mb_len);
		break;
	default:
		break;
	}
}

void modbus_init()
{
	modbus_temp_buf[0] = 0x5a;
	modbus_temp_buf[1] = 0xa5;
	modbus_temp_buf[2] = 0x0a;

}


void modbus_task()
{

	//readLen = iot_uart485_buffer_len();
	//readLen = iot_uart485_recv(modbus_temp_buf);
	uint16_t i;
	uint8_t sum = 0;

	if (modbus_temp_len)
	{

		//校验
		for(i=0;i<modbus_temp_len-1;i++)
		{
			sum += modbus_temp_buf[i];
		}

		if(sum ==modbus_temp_buf[i])
		{
			dispatch_modbus(modbus_temp_buf);
		}else
		{
#ifdef DEBUG
		SEGGER_RTT_printf(0,"verify sum faid\n");
#endif
		}
		modbus_temp_len = 0;
	}
}
