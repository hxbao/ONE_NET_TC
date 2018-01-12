/*
 * misc.c
 *
 *  Created on: 2018��1��9��
 *      Author: hxbao
 */

#include <mymisc.h>
#include <stdint.h>
#include <string.h>
#include "timer.h"

uint8_t ischar(char a)
{
	if (('0' <= a && a <= '9') || ('a' <= a && a <= 'f')
			|| ('A' <= a && a <= 'F'))
	{
		return 1;
	}
	else
		return 0;
}

uint8_t asc2hex(char a)
{
	uint8_t ret;
	if ('0' <= a && a <= '9')
		ret = a - '0';
	else if ('a' <= a && a <= 'f')
		ret = a - 'a' + 10;
	else if ('A' <= a && a <= 'F')
		ret = a - 'A' + 10;
	else
		ret = 0;

	return ret;

}

int8_t hex2asc(uint8_t hex, char *charlow, char *charhigh)
{
	uint8_t tmphex;
	char tmp = 0;

	tmphex = hex & 0x0f;

	if (tmphex < 10)
	{
		tmp = tmphex + '0';
		*charlow = tmp;
	}
	else if (tmphex < 16)
	{
		tmp = tmphex - 10 + 'A';
		*charlow = tmp;
	}
	else
	{
		return -1;
	}

	tmphex = (hex & 0xf0) >> 4;
	if (tmphex < 10)
	{
		tmp = tmphex + '0';
		*charhigh = tmp;
	}
	else if (tmphex < 16)
	{
		tmp = tmphex - 10 + 'A';
		*charhigh = tmp;
	}
	else
	{
		return -1;
	}

	return 0;
}

//�����ַ�ת����һ��ʮ�������ֽ�
uint8_t asc2hex2(char *a2)
{
	uint8_t ret = 0;
	if (strlen(a2) >= 2)
	{
		ret = asc2hex(*a2) << 4;
		ret |= asc2hex(*(a2 + 1));
	}

	return ret;
}

//��һ��16�����ַ���ת����ʮ�������ֽ�����
int8_t asc2hex3(char *a3, uint16_t len, uint8_t *pOutBuffer)
{
	//������һ���������ַ�������˫�����򲻷���ת��
	if (len % 2)
	{
		return -1;
	}
	for (uint16_t i = 0; i < len / 2; i++)
	{
		*(pOutBuffer + i) = asc2hex2(a3 + 2 * i);
	}
	return 0;
}

int8_t hex2ascString(uint8_t *psrc, char *pdes, uint16_t len)
{
	uint16_t i;
	char charl;
	char charh;

	for (i = 0; i < len; i++)
	{
		if (hex2asc(*(psrc + i), &charl, &charh))
		{
			return -1;
		}
		*(pdes + 2 * i) = charh;
		*(pdes + 2 * i + 1) = charl;
	}
	*(pdes + 2 * len) = 0;
	return 0;
}

void sys_stop_delay(uint32_t ms)
{
	uint16_t count = 0;
	while (1)
	{
		timer_sleep(100);
		count++;
		if (100 * count > ms)
		{
			break;
		}
	}
}
