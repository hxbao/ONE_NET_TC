/*
 * test_spi.c
 *
 *  Created on: 2017年12月1日
 *      Author: hxbao
 */
#include "test.h"

void test_spi_master()
{
	uint8_t data = 0x55;
	spi1_master_hal_init();
	while (1)
	{
		spi1_master_write_bytes(&data,1);
		timer_sleep(100);
	}
}

void test_spi_slave()
{
	spi_slave_hal_init();
	spi_recv_finish_exit_config();

	while (1)
	{
		if (commd_cur_len > 0)
		{
#ifdef DEBUG
			trace_printf("GET spi slave command->%s\n", commd_buf);
#endif			//原样返回数据
			//spi_slave_tx_data(commd_buf,commd_cur_len);
			commd_cur_len = 0;
		}
	}
}

