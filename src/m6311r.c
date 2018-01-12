/*
 * m6311r.c
 *
 *  Created on: 2018年1月5日
 *      Author: hxbao
 */

//#include "stm32f10x.h"
#include <mymisc.h>
#include "uart.h"
#include "string.h"
#include "stdio.h"
#include "timer.h"
#include "mymemery.h"
#include "g_conf.h"
#include <stdlib.h>
#include "commd_if.h"
#include "diag/trace.h"
#include "modbus.h"


#pragma GCC diagnostic ignored "-Wunused-function"

#define M6311_USE_USARTX USART3

#define iot_uart_buffer_len() Uart3RxCount

char M6311_IMEI_id[20] =
{ 0 };
char M6311_SIM_id[25] =
{ 0 };

//onenet 连接状态
uint8_t one_connect_state = 0;

//内部使用函数
uint8_t get_info_from_m6311(char* cmd, char *ret, uint8_t retLen,
		int32_t timeoutms);
uint8_t get_simid(char *simid);
uint8_t get_imeiid(char *imeiid);
uint8_t get_csq();
uint8_t check_onenet_connect_status();
uint8_t iot_onenet_send_raw(uint8_t *databuf, uint8_t rawNum, uint16_t dataLen);

void iot_uart_send(uint8_t *pBuf, uint16_t size)
{
	uart_send_data(M6311_USE_USARTX, pBuf, size);
}

//只要数据被读取之后,uart接收数据会重新存储
void iot_uart_recv(uint8_t *pBuf, uint16_t size)
{
	uart_read(M6311_USE_USARTX, pBuf, size);
}

//Check AT command return value
uint8_t iot_wait_at_ack(char *pAck, uint32_t timeOutMs)
{
	uint8_t *recvBuf = my_malloc(256);
	uint32_t dataLen = 0;
	uint8_t timeCount = 0;

	if (!pAck)
	{
#ifdef DEBUG
		trace_printf("ack buffer is null");
#endif
		return 1;
	}
	//开始计时
	timer_sleep(100);
	do
	{
		dataLen = iot_uart_buffer_len();
		//判定接收到的数据已经足以判定是否达到预期的结果的字节数
		if (dataLen > strlen(pAck))
		{

			iot_uart_recv(recvBuf, dataLen);
			*(recvBuf + dataLen) = 0;

			if (strstr((char*) recvBuf, (char*) pAck) != NULL)
			{
#ifdef DEBUG
				trace_printf("get ack:%s\n", recvBuf);
#endif
				uart_fflush(M6311_USE_USARTX);
				my_free(recvBuf);
				return 0;
			}

		}
		timer_sleep(100);
		timeCount++;

		//超时退出
		if ((uint32_t) (timeCount * 100 + 100) > timeOutMs)
		{
#ifdef DEBUG
			if(recvBuf !=NULL)
			{
				trace_printf("get ack:%s\n", recvBuf);
			}else
			{
				trace_printf("get ack:NULL\n");
			}

#endif
			break;
		}

	} while (1);
	my_free(recvBuf);
	return 1;
}

int32_t iot_send_at_cmd(char *pCmd, char *pAck, int32_t timeOutMs)
{
	uart_fflush(M6311_USE_USARTX);
#ifdef DEBUG
	trace_printf("send at commad:%s\n", pCmd);
#endif
	iot_uart_send((uint8_t*) pCmd, strlen((char*) pCmd));
	if (iot_wait_at_ack(pAck, timeOutMs))
	{
#ifdef DEBUG
		trace_printf("get at commad timeout\n", pCmd);
#endif
		return 1;
	}

	return 0;
}

void m6311r_reset()
{
	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_6;
	gpio.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &gpio);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	sys_stop_delay(3000);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);
}

void gprs_connect()
{
	//iot_send_at_cmd("AT+CREG=0\r\n", "OK", 1000);
	while (1)
	{
		//如果没有注册网络，等待
		if (iot_send_at_cmd("AT+CREG?\r\n", "+CREG: 1,1", 1000) == 0)
		{
			break;
		}
		timer_sleep(1000);
	}
//	//查询信号质量
	iot_send_at_cmd("AT+CSQ\r\n", "OK", 10000);
//	//APN接入
	iot_send_at_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n", "OK", 10000);
//	//附着GPRS
//	iot_send_at_cmd("AT+CGATT=1\r\n", "OK", 10000);
	//附着PDP
	iot_send_at_cmd("AT+CGACT=1,1\r\n", "OK", 10000);
	//得到imei号
	get_imeiid(M6311_IMEI_id);
	//得到iccid
	get_simid(M6311_SIM_id);

}

void onenet_init()
{
	char cmd[128];

	if (iot_send_at_cmd("AT+CIOT?\r\n", "OK", 5000))
	{
		//设置产品id
		snprintf(cmd, sizeof(cmd), "AT+CIOTPROID=%s\r\n", ONENET_PRODUCT_ID);
		iot_send_at_cmd(cmd, "OK", 3000);
		timer_sleep(500);
		//config mkey for onenet
		snprintf((char*) cmd, sizeof(cmd), "AT+CIOTMKEY=\"%s\"\r\n",
		ONENET_MKEY);
		iot_send_at_cmd(cmd, "OK", 3000);
		timer_sleep(500);
		//设置鉴权信息
		snprintf((char*) cmd, sizeof(cmd), "AT+CIOTAUTHINFO=\"%s\"\r\n",
				M6311_IMEI_id);
		iot_send_at_cmd(cmd, "OK", 3000);
		timer_sleep(500);
		//设置设备描述信息
		snprintf((char*) cmd, sizeof(cmd), "AT+CIOTINIT=\"%s\",\"%s\"\r\n",
				M6311_IMEI_id, ONENET_DESP);
		iot_send_at_cmd(cmd, "OK", 3000);
		timer_sleep(500);
		//执行设备初始化
		iot_send_at_cmd("AT+CIOTINIT\r\n", "OK", 5000);
		timer_sleep(500);
	}
	//set connect param
	iot_send_at_cmd("AT+CIOTCONNECTPARA=1\r\n", "OK", 5000);
	timer_sleep(500);

	if (iot_send_at_cmd("AT+CIOTSTART=0\r\n", "CONNECT", 10000))
	{
		one_connect_state = 0;
	}
	else
	{
		one_connect_state = 1;
	}

}

uint8_t iot_onenet_send_ping()
{
	static uint8_t reConnectCount = 0;

	if (iot_send_at_cmd("AT+CIOTPING\r\n", "+CIOTPING: OK", 5000))
	{
		if (check_onenet_connect_status() == 0)
		{
			//need reconnect to onenet
			iot_send_at_cmd("AT+CIOTSTART=0\r\n", "CONNECT OK", 5000);

			reConnectCount++;
			if (reConnectCount >= 3)
			{
				reConnectCount = 0;
				one_connect_state = 0;
				//reset_gprs_module();
				gprs_connect();
				onenet_init();
			}
		}

	}
	else
	{
		reConnectCount = 0;
		one_connect_state = 1;
	}
	return 0;
}

uint8_t iot_onenet_send_bin_data(uint8_t *databuf, uint16_t dataLen)
{
	uint16_t len;


	len = 2 * dataLen + strlen("AT+CIOTDAT=0,%d,\"%s\"\r\n") + 10;

	uint8_t *pDatabuf = my_malloc(len);
	uint8_t *binData = my_malloc(2 * dataLen + 10);

	if ((binData == NULL) && (pDatabuf == NULL))
	{
		goto here;
	}

	hex2ascString(databuf, (char*) binData, dataLen);
	snprintf((char*) pDatabuf, len, "BIN_DATA,,%s", binData);

	//检查是否在线状态
	if (check_onenet_connect_status() == 0)
	{
		//need reconnect to onenet
		iot_send_at_cmd("AT+CIOTSTART=0\r\n", "CONNECT OK", 5000);
	}

	if (iot_onenet_send_raw((uint8_t*) pDatabuf, 1, strlen((char*) pDatabuf)))
	{
		goto here;
	}

//////////////////////////////////////////////

	my_free(binData);
	my_free(pDatabuf);
	return 0;

	here: my_free(binData);
	my_free(pDatabuf);
	return 1;
}

void iot_send_simcard_id()
{
	char pCmdBuf[50];

	if (strlen(M6311_SIM_id) < 16)
	{	//get simid is successfull
		get_simid(M6311_SIM_id);
	}
	snprintf(pCmdBuf, 50, "iccid,,%s", M6311_SIM_id);
	iot_onenet_send_raw((uint8_t*) pCmdBuf, 1, strlen(pCmdBuf));
}

void iot_send_csq()
{
	char pCmdBuf[50];
	uint8_t csq = 0;
	csq = get_csq();
	snprintf(pCmdBuf, 50, "csq,,%d", csq);
	iot_onenet_send_raw((uint8_t*) pCmdBuf, 1, strlen(pCmdBuf));
}

//void iot_send_location_to_onenet()
//{
//	char *s1 = "lat %f lon %f speed %f ";
//	char *s2= "location,,";
//	uint16_t len2 = strlen(s1)+5*3+2;
//	uint16_t len = strlen(s2)+strlen(s1)*6+3*5*6+10;
//	char *pCmdBuf = pvPortMalloc(len);
//	char *tmpstr = pvPortMalloc(len2);
//
//	if(pCmdBuf ==NULL || tmpstr == NULL)
//	{
//		return ;
//	}
//	*pCmdBuf = 0;
//	strcat(pCmdBuf,s2);
//	snprintf(tmpstr,len2,s1,gps.lat,gps.lon,gps.speed);
//	strcat(pCmdBuf,tmpstr);
//
//	iot_onenet_send_raw((uint8_t*)pCmdBuf,1,strlen(pCmdBuf));
//	vPortFree(pCmdBuf);
//	vPortFree(tmpstr);
//}

//+CIOTCMD=<datalen>,<data>
uint8_t iot_onenet_read(uint8_t *pRecvBuffer)
{
	uint8_t * uartMsg;
	int32_t uartMsgLen = 0;
	int32_t msgLen = 0;
	char *onenetCmdMsg = NULL;
	int8_t tmp[8] =
	{ 0 };
	int32_t pos = 0;

	uartMsgLen = iot_uart_buffer_len();

	if (uartMsgLen > 10)
	{
		//等待数据全部接收完
		sys_stop_delay(500);
		uartMsgLen = iot_uart_buffer_len();
		uartMsg = my_malloc(uartMsgLen); //预先分配足够空间
		if (uartMsg == NULL)
		{
			return -1;
		}
		//读取数据
		iot_uart_recv(uartMsg, uartMsgLen);
		onenetCmdMsg = 0;
		//搜寻"+CIOTCMD"
		onenetCmdMsg = strstr((char*) uartMsg, "+CIOTCMD:");

		if (onenetCmdMsg)
		{

			onenetCmdMsg += 9;
			while ((*onenetCmdMsg) != ',')
			{
				tmp[pos] = *onenetCmdMsg;
				pos++;
				onenetCmdMsg++;
			}
			onenetCmdMsg++;
			msgLen = atoi((char*) tmp);

			if (msgLen)
			{
				memcpy(pRecvBuffer, onenetCmdMsg, msgLen);
				uart_fflush(M6311_USE_USARTX);
				//add \0
				*(pRecvBuffer + msgLen) = 0;
				my_free(uartMsg);
				return msgLen;
			}
		}
		my_free(uartMsg);
	}
	return 0;
}

//最大支持1024的命令下发
void iot_onenet_task(uint32_t count)
{
	uint16_t readLen = 0;
	uint8_t *pRecvBuffer = my_malloc(2048);
	if (pRecvBuffer != NULL)
	{
		//从网络读数据
		readLen = iot_onenet_read(pRecvBuffer);
		if (readLen)
		{
#ifdef DEBUG
			trace_printf("get onenet data:%d,%s\n", readLen, pRecvBuffer);
#endif
			memcpy(commd_buf, pRecvBuffer, readLen);
			commd_cur_len = readLen;
			commd_buf[commd_cur_len] = 0;
			get_commd();
			//commd_poll();
		}
		my_free(pRecvBuffer);
	}
	//写数据到网络
	if (IsWrite485Buffer) //如果有485写入的数据上传到平台
	{
		lasttimeSendData2OnenetStu = iot_onenet_send_bin_data(modbus_writeBuf,
				IsWrite485Buffer);
		stu_count = 1;
		IsWrite485Buffer = 0;
	}
	else
	{
		if (count % 120 == 1)
		{
			iot_onenet_send_ping();
		}
	}
}

//---------------------------------------------------------------------------------------------

uint8_t get_info_from_m6311(char* cmd, char *ret, uint8_t retLen,
		int32_t timeoutms)
{
	uint16_t dataLen;
	uint8_t timeCount = 0;

	uart_fflush(M6311_USE_USARTX);
	iot_uart_send((uint8_t*) cmd, strlen(cmd));
	do
	{
		dataLen = iot_uart_buffer_len();
		if (dataLen >= retLen)
		{
			iot_uart_recv((uint8_t*) ret, dataLen);
			uart_fflush(M6311_USE_USARTX);
			return 0;
		}
		else
		{
			timer_sleep(100);
			timeCount++;
		}

		//超时退出
		if ((timeCount * 100 + 100) > timeoutms)
		{
			break;
		}
	} while (1);
	uart_fflush(M6311_USE_USARTX);
	return 1;
}

uint8_t get_simid(char *simid)
{
	char *p1;

	uint16_t dataLen;

	uint8_t *recvBuf = my_malloc(125);
	dataLen = strlen("+CCID: 1,\"9868200B62510C200065\"");

	if (get_info_from_m6311("AT+CCID?\r\n", (char*) recvBuf, dataLen, 2000))
	{
#ifdef DEBUG
		trace_printf("get ccid timeout\n");
#endif
		return 1;
	}
#ifdef DEBUG
	//trace_printf("get simid recvBuf:%s\n",recvBuf);
#endif
	p1 = strstr((char*) recvBuf, "+CCID");
	if (p1 != NULL)
	{
		p1 += strlen("+CCID: 0,\"");
		sscanf(p1, "%[0123456789ABCDEF]", simid);
#ifdef DEBUG
		trace_printf("get simid:%s\n", simid);
#endif
		my_free(recvBuf);
		return 0;
	}
	my_free(recvBuf);
	return 1;
}

uint8_t get_imeiid(char *imeiid)
{
	char *p1;

	uint16_t dataLen;

	uint8_t *recvBuf = my_malloc(125);

	dataLen = strlen("+CGSN: 1,012345678901234");

	if (get_info_from_m6311("AT+CGSN\r\n", (char*) recvBuf, dataLen, 10000))
	{
#ifdef DEBUG
		trace_printf("get imei timeout\n");
#endif
		return 1;
	}
#ifdef DEBUG
	//trace_printf("get imei recvBuf:%s\n",recvBuf);
#endif
	p1 = strstr((char*) recvBuf, "+CGSN");
	if (p1 != NULL)
	{
		p1 += strlen("+CGSN: 1,");
		sscanf(p1, "%[0123456789ABCDEF]", imeiid);
#ifdef DEBUG
		trace_printf("get imeiid:%s\n", imeiid);
#endif
		my_free(recvBuf);
		return 0;
	}
	my_free(recvBuf);
	return 1;
}

uint8_t get_csq()
{
	char *p1;
	int csq;

	uint16_t dataLen;

	uint8_t *recvBuf = my_malloc(48);

	dataLen = strlen("+CSQ:23,99");

	if (get_info_from_m6311("AT+CSQ\r\n", (char*) recvBuf, dataLen, 1000))
	{
#ifdef DEBUG
		trace_printf("get csq timeout\n");
#endif
		return 1;
	}
#ifdef DEBUG
	trace_printf("get csq ack:%s\n", recvBuf);
#endif
	p1 = strstr((char*) recvBuf, "+CSQ:");
	if (p1 != NULL)
	{
		sscanf(p1, "+CSQ:%d", &csq);
		my_free(recvBuf);
		return (uint8_t) csq;
	}
	my_free(recvBuf);
	return 1;
}

uint8_t check_onenet_connect_status()
{
	char *p1;
	int iotstatus;

	uint16_t dataLen;

	uint8_t *recvBuf = my_malloc(48);

	dataLen = strlen("+CIOTSTATUS: 1");

	if (get_info_from_m6311("AT+CIOTSTATUS\r\n", (char*) recvBuf, dataLen,
			2000))
	{
#ifdef DEBUG
		trace_printf("get CIOTSTATUS timeout\n");
#endif
		return 1;
	}

	p1 = strstr((char*) recvBuf, "+CIOTSTATUS:");
	if (p1 != NULL)
	{
		sscanf(p1, "+CIOTSTATUS: %d", &iotstatus);
#ifdef DEBUG
		trace_printf("get ack:%s", (char*) recvBuf);
#endif
		my_free(recvBuf);
		return (uint8_t) iotstatus;
	}
	my_free(recvBuf);
	return 1;
}

uint8_t iot_onenet_send_raw(uint8_t *databuf, uint8_t rawNum, uint16_t dataLen)
{
	uint16_t len = dataLen + strlen("AT+CIOTDAT=0,%d,\"%s\"\r\n") + 10;

	uint8_t *pDatabuf = my_malloc(len);

	if (pDatabuf != NULL)
	{
		snprintf((char*) pDatabuf, len, "AT+CIOTDAT=0,%d,\"%s\"\r\n", rawNum,
				(char*) databuf);
	}
	else
	{
#ifdef DEBUG
		trace_printf("my_malloc failed in iot_onenet_send_raw\n");
#endif
		return 1;
	}

//	if (iot_send_at_cmd((char*) pDatabuf, "OK", 5000))
//	{
//
//		goto return_here;
//	}
	//尽可能快退出,不然有吃掉上面下发的数据指令
	iot_send_at_cmd((char*) pDatabuf, "OK", 2000);

	my_free(pDatabuf);
	return 0;
}

