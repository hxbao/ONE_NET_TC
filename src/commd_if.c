/*
 * commd_if.c
 *
 *  Created on: 2017年11月28日
 *      Author: hxbao
 */
#include <string.h>
#include "stdio.h"
#include "commd_if.h"
#include "stm32f10x.h"
#include "spi.h"
#include "uart.h"
#include "mymemery.h"
#include "diag/Trace.h"
#include "lowPowerCtrl.h"
#include "g_conf.h"
#include "timer.h"
#include "m6311r.h"
#include "eeprom.h"

//接收的命令缓冲
uint8_t commd_buf[COMMD_BUF_SIZE];
//当前 接收的命令的长度
uint16_t commd_cur_len;

uint8_t diableCommdPoll = 0;

//需要存储的指令结构
typedef struct
{
	uint8_t cmdId;        //指令id
	uint8_t isActived;
	int16_t cmdRunTimes; //指令执行的有限次数
	uint32_t cmdRunInteval; //指令执行的间隔时间（ms）
	char cmd[128];
} CMD_t;

//共存储10条指令
CMD_t cmds[10];

#define CMD_START_SAMPLE 0x01
#define CMD_STOP_SAMPLE 0x02
#define CMD_CONFIG_HZ    0x03
#define CMD_CONFIG_ROLA_ADDR 0x04
#define CMD_CONFIG_DEV_INFO 0x05
#define CMD_GET_DEV_INFO 0x06
#define CMD_STOP_ROLA_SEND 0x07
#define CMD_GET_BLOCK_DATA 0x08

//模块内部函数
void get_at_cmd_para(char* pAT, CMD_t* c);
void rm_at_cmd_para(char* pAT, CMD_t* c);

//从命令缓冲中解析得到应用命令和参数
uint8_t get_commd(uint8_t* commd, uint8_t argv[])
{
	if (diableCommdPoll)
	{
		return 0;
	}

	uint16_t dataLen = 0;
	dataLen = strlen((char*) commd_buf);
	uint8_t *pUart485SendBuf;

	if (commd_cur_len > 0)
	{
#ifdef DEBUG
		trace_printf("GET command->%s\n", commd_buf);
#endif

		if (strstr((char*) commd_buf, "AT+CMDDOWNCYC") != NULL)
		{
			get_at_cmd_para((char *) commd_buf, cmds);
			//store to flash
		}
		else if (strstr((char*) commd_buf, "AT+CMDRM") != NULL)
		{
			rm_at_cmd_para((char *) commd_buf, cmds);
			//store to flash
		}
		else if (strstr((char*) commd_buf, "AT+CMDGET_SIMID") != NULL)
		{
			iot_send_simcard_id();
		}
		else if (strstr((char*) commd_buf, "AT+CMDSET_DEVICEID") != NULL)
		{

		}
		else if (strstr((char*) commd_buf, "AT+CMDGET_ACCDATA") != NULL)
		{

		}
		else if (strstr((char*) commd_buf, "AT+CMDGET_LOCATION") != NULL)
		{

		}
		else if (strstr((char*) commd_buf, "AT+CMDSETGPS=") != NULL)
		{

		}
		else if (strstr((char*) commd_buf, "AT+CMDSET_RESET") != NULL)
		{

		}
		else if (strstr((char*) commd_buf, "AT+CMDGET_CSQ") != NULL)
		{
			iot_send_csq();

		}
		else if (strstr((char*) commd_buf, "AT+CMD485BAUD=") != NULL)
		{

		}
		else
		{
			//直接转发数据

			pUart485SendBuf = my_malloc(dataLen / 2);
			asc2hex3((char*) commd_buf, dataLen, pUart485SendBuf);
			//立即485转发
			//iot_uart485_send(pUart485SendBuf,len/2);
			//写入modbus 读缓冲区，等待被读取，如果没有被及时读取，覆盖旧的缓冲数据
			memcpy(modbus_readBuf, pUart485SendBuf, dataLen / 2);
			my_free(pUart485SendBuf);
			readBufCnt = dataLen / 2;
		}

		commd_cur_len = 0;
		return 1;
	}
	return 0;

}

void commd_poll()
{
	uint8_t cmd;
	uint8_t argv[16];
	if (get_commd(&cmd, argv))
	{
#ifdef DEBUG
		trace_printf("cmd:%d,argv1:%d,arg2:%d\n", cmd, *(int*) &argv,
				*(int*) &argv[4]);
#endif

		switch (cmd)
		{
		case CMD_START_SAMPLE:

			break;
		case CMD_STOP_SAMPLE:

			break;
		case CMD_CONFIG_HZ:

			break;
		case CMD_CONFIG_ROLA_ADDR:

			break;
		case CMD_CONFIG_DEV_INFO:

			break;
		case CMD_GET_DEV_INFO:

			break;
		case CMD_STOP_ROLA_SEND:

			break;
		case CMD_GET_BLOCK_DATA:

			break;
		}

	}
}

void poll_store_commd()
{
	static uint16_t loopCount = 0;
	for (uint8_t i = 0; i < 10; i++)
	{
		if (cmds[i].isActived == 1)
		{
			if (loopCount % (cmds[i].cmdRunInteval) == 0)
			{
				if (cmds[i].cmdRunTimes != 0 || cmds[i].cmdRunTimes == -1)
				{
					//cmds[i].cmdRunTimes == -1 是永远执行指令
					if (cmds[i].cmdRunTimes != -1)
					{
						cmds[i].cmdRunTimes--;
					}

					uint16_t dataLen = strlen(cmds[i].cmd);
					uint8_t *pUart485SendBuf = my_malloc(dataLen / 2);

					asc2hex3(cmds[i].cmd, strlen(cmds[i].cmd), pUart485SendBuf);
					//立即485转发
					//iot_uart485_send(pUart485SendBuf, dataLen / 2);
					//check receive，and wait 1000ms

					my_free(pUart485SendBuf);
				}
			}
		}
	}
	loopCount++;
}

//得到周期性下发指令的参数,并存储到cmds中
void get_at_cmd_para(char* pAT, CMD_t* c)
{
	int id;
	int runcount;
	int timeInterval;
	char cmd[128];

	sscanf(pAT, "AT+CMDDOWNCYC=%d,%d,%d,\"%[0123456789abcdefABCDEF]\"\r\n", &id,
			&runcount, &timeInterval, cmd);
	c[id].cmdId = id;
	c[id].cmdRunTimes = runcount;
	c[id].cmdRunInteval = timeInterval;
	strcpy(c[id].cmd, cmd);
	c[id].isActived = 1;
}

void rm_at_cmd_para(char* pAT, CMD_t* c)
{
	int id;

	sscanf(pAT, "AT+CMDRM=%d\r\n", &id);

	if (id < 10 && id >= 0)
	{
		c[id].isActived = 0;
	}

}

void store_cmd_para_to_flash_task()
{
	uint8_t i, j;
	uint8_t* buf = my_malloc(sizeof(CMD_t));
	//读取flash存储的命令数据与现存内存中的数据作比对
	for (i = 0; i < 10; i++)
	{
		stmflash_read_bytes(STMFLASH_BASE + i * STMFLASH_PARA_OFFSET,
				(uint16_t*) buf, sizeof(CMD_t) / 2);
		//比较,如果不一致,存储到flash中
		for (j = 0; j < sizeof(CMD_t); j++)
		{
			if (*((uint8_t*) &cmds[i] + j) != *(buf + j))
			{
				//不一致,存储到flash
				stmflash_write(STMFLASH_BASE + i * STMFLASH_PARA_OFFSET,
						(uint16_t*) &cmds[i], sizeof(CMD_t) / 2);
				break;
			}
		}

	}
	my_free(buf);

}

void load_cmd_para_from_flash()
{
	uint8_t i;

	for (i = 0; i < 10; i++)
	{
		stmflash_read_bytes(STMFLASH_BASE + i * STMFLASH_PARA_OFFSET,
				(uint16_t*) &cmds[i], sizeof(CMD_t) / 2);

	}
}

#define SAMPLE_DATA_BLOCK_SIZE 512
void task_todo()
{

}
