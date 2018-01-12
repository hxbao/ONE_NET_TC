/*
 * test_mymemery.c
 *
 *  Created on: 2017Äê12ÔÂ1ÈÕ
 *      Author: hxbao
 */

#include "test.h"

void test_mymemery()
{
	uint16_t i;
	uint8_t *pbuf, *pbuf1;
	pbuf = my_malloc(1024);

	for (i = 0; i < 1024; i++)
	{
		*(pbuf + i) = 0x55;
	}

	my_free(pbuf);

	pbuf = my_malloc(256);

	for (i = 0; i < 256; i++)
	{
		*(pbuf + i) = 0xaa;
	}

	pbuf1 = my_malloc(4096);
	if (pbuf1 != NULL)
	{
		for (i = 0; i < 4096; i++)
		{
			*(pbuf1 + i) = 0xbb;
		}
	}
	my_free(pbuf1);

	my_free(pbuf);
}
