/*
 * stmflash.c
 *
 *  Created on: 2017年11月28日
 *      Author: hxbao
 */
#include "stm32f10x.h"
#include "mymemery.h"
#include "eeprom.h"

#define STM32_FLASH_BASE 0x08000000
#define STM_SECTOR_SIZE 1024 //字节
#define STM32_FLASH_SIZE 128

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void stmflash_read_bytes(uint32_t stmFlashAddr, uint16_t *pbuf,
		uint16_t size)
{
	uint16_t i;
	for (i = 0; i < size; i++)
	{
		pbuf[i] = *(__IO uint16_t*) stmFlashAddr; //读取2个字节.
		stmFlashAddr += 2; //偏移2个字节.
	}
}

void stmflash_write_bytes_nocheck(uint32_t stmFlashAddr, uint16_t *pbuf, uint16_t size)
{
	uint16_t i;
	for (i = 0; i < size; i++)
	{
		FLASH_ProgramHalfWord(stmFlashAddr, *(pbuf + i));
		stmFlashAddr += 2; //地址增加2.
	}
}

//WriteAddr: 起始地址
//pBuffer:   数据指针
//NumToWrite:半字(16位)数
void stmflash_write(uint32_t stmFlashAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
	uint16_t i;

	uint32_t secpos;	   //扇区地址
	uint16_t secoff;	   //扇区内偏移地址(16位字计算)
	uint16_t secremain;  //扇区内剩余地址(16位字计算)
	uint32_t offaddr;    //去掉0X08000000后的地址

	uint16_t *buf = (uint16_t*)my_malloc(STM_SECTOR_SIZE*2);

	if(buf ==0)
	{
		return;
	}

	if (stmFlashAddr < STM32_FLASH_BASE
			|| (stmFlashAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
		return;    //非法地址

	offaddr = stmFlashAddr - STM32_FLASH_BASE;		    //实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;			 //扇区地址  0~127 for STM32F103RBT6
	secoff = (offaddr % STM_SECTOR_SIZE) / 2;		    //在扇区内的偏移(2个字节为基本单位.)
	secremain = STM_SECTOR_SIZE / 2 - secoff;		        //扇区剩余空间大小
	if (NumToWrite <= secremain)
		secremain = NumToWrite;  //不大于该扇区范围
	FLASH_Unlock();
	while (1)
	{
		stmflash_read_bytes(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, buf,
				STM_SECTOR_SIZE / 2);  //读出整个扇区的内容

		for (i = 0; i < secremain; i++)  //校验数据
		{
			if (*(buf+secoff + i) != 0XFFFF)
				break;  //需要擦除
		}
		if (i < secremain)  //需要擦除
		{
			//擦除页
			FLASH_ErasePage(secpos * STM_SECTOR_SIZE+ STM32_FLASH_BASE);

			for (i = 0; i < secremain; i++)  //复制
			{
				*(buf+secoff + i) = pBuffer[i];
			}

			stmflash_write_bytes_nocheck(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE,
					buf, STM_SECTOR_SIZE / 2);  //写入整个扇区

		}
		else
			stmflash_write_bytes_nocheck(stmFlashAddr, pBuffer, secremain); //写已经擦除了的,直接写入扇区剩余区间.
		if (NumToWrite == secremain)
			break;  //写入结束了
		else  //写入未结束
		{
			secpos++;				//扇区地址增1
			secoff = 0;				//偏移位置为0
			pBuffer += secremain;  	//指针偏移
			stmFlashAddr += secremain;	//写地址偏移
			NumToWrite -= secremain;	//字节(16位)数递减
			if (NumToWrite > (STM_SECTOR_SIZE / 2))
				secremain = STM_SECTOR_SIZE / 2;	//下一个扇区还是写不完
			else
				secremain = NumToWrite;	//下一个扇区可以写完了
		}
	}
	FLASH_Lock();
	my_free((uint8_t*)buf);

}

