/*
 * spi.h
 *
 *  Created on: 2017年11月28日
 *      Author: hxbao
 */

#ifndef INCLUDE_SPI_H_
#define INCLUDE_SPI_H_
#include "stm32f10x.h"

#define flash_spi_set_cs_high() GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define flash_spi_set_cs_low() GPIO_ResetBits(GPIOB,GPIO_Pin_12)


//作为spi slave 被动模块初始化
void spi_slave_hal_init(void);
void spi_master_hal_init(void);
uint8_t spi_master_read_bytes(uint8_t *buff, int size);
uint8_t spi_master_write_bytes(uint8_t *buff, int size);

void spi1_master_hal_init(void);
uint8_t spi1_master_read_bytes(uint8_t *buff, int size);
uint8_t spi1_master_write_bytes(uint8_t *buff, int size);

void spi_recv_finish_exit_config();
uint8_t spi_slave_tx_data(uint8_t *pdata,uint8_t len);


//uint8_t spi_master_read_write_byte(uint8_t txByte);




#endif /* INCLUDE_SPI_H_ */
