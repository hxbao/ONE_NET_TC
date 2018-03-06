/*
 * eeprom.h
 *
 *  Created on: 2017年12月15日
 *      Author: hxbao
 */

#ifndef INCLUDE_EEPROM_H_
#define INCLUDE_EEPROM_H_

#include "stm32f10x.h"
#include "g_conf.h"

#define EEPROM_BASE_ADDR 0x08080000
#define EEPROM_BYTE_SIZE 0x0FFF

//每个结构对象保留256字节的空间
#define EEPROM_PARA1_BASE (0)
#define EEPROM_PARA2_BASE (0x100)
#define EEPROM_PARA3_BASE (0x200)
#define EEPROM_PARA4_BASE (0x300)
#define EEPROM_PARA5_BASE (0x400)
#define EEPROM_PARA6_BASE (0x500)
#define EEPROM_PARA7_BASE (0x600)
#define EEPROM_PARA8_BASE (0x700)
#define EEPROM_PARA9_BASE (0x800)
#define EEPROM_PARA10_BASE (0x900)
#define EEPROM_PARA11_BASE (0xa00)
#define EEPROM_PARA12_BASE (0xb00)
#define EEPROM_PARA13_BASE (0xc00)
#define EEPROM_PARA14_BASE (0xd00)
#define EEPROM_PARA15_BASE (0xe00)

//定义内部flash保存最后一块参数的地址偏移,最后4K保存指令空间
#define STMFLASH_BASE    (0x08000000+60*1024)
#define STMFLASH_PARA_OFFSET (0x100)
#define STMFLASH_PARA1_BASE (0)
#define STMFLASH_PARA2_BASE (0x100)
#define STMFLASH_PARA3_BASE (0x200)
#define STMFLASH_PARA4_BASE (0x300)
#define STMFLASH_PARA5_BASE (0x400)
#define STMFLASH_PARA6_BASE (0x500)
#define STMFLASH_PARA7_BASE (0x600)
#define STMFLASH_PARA8_BASE (0x700)
#define STMFLASH_PARA9_BASE (0x800)
#define STMFLASH_PARA10_BASE (0x900)
#define STMFLASH_PARA11_BASE (0xa00)
#define STMFLASH_PARA12_BASE (0xb00)
#define STMFLASH_PARA13_BASE (0xc00)
#define STMFLASH_PARA14_BASE (0xd00)
#define STMFLASH_PARA15_BASE (0xe00)
//#define STMFLASH_PARA16_BASE (0xf00)

#define STMFLASH_IAP_INFO_BASE (0xf00)


void stmflash_read_bytes(uint32_t stmFlashAddr, uint16_t *pbuf,
		uint16_t size);
void stmflash_write(uint32_t stmFlashAddr, uint16_t *pBuffer, uint16_t NumToWrite);



void EEPROM_ReadBytes(uint16_t Addr, uint8_t *Buffer, uint16_t Length);
void EEPROM_ReadWords(uint16_t Addr, uint16_t *Buffer, uint16_t Length);
void EEPROM_WriteBytes(uint16_t Addr, uint8_t *Buffer, uint16_t Length);

#endif /* INCLUDE_EEPROM_H_ */
