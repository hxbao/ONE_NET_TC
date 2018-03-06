/*
 * iap.c
 *
 *  Created on: 2018年3月6日
 *      Author: hxbao
 */

#include "eeprom.h"
#include "iap.h"
#include "stm32f10x.h"

//#define FLASH_START_ADDR_APP2 0x8007FFF;

//跳转入口函数指针声明
iapfun jump2app;

uint32_t fwaddr;
uint16_t binDataCrc = 0;
uint32_t binDataTotalSize = 0;

uint16_t iapbuf[512];
Iap_BlockData_def bd;
AppBinHandle_t appBin;

void InvertUint8(uint8_t *dBuf, uint8_t *srcBuf)
{
	int i;
	unsigned char tmp[4];
	tmp[0] = 0;
	for (i = 0; i < 8; i++)
	{
		if (srcBuf[0] & (1 << i))
			tmp[0] |= 1 << (7 - i);
	}
	dBuf[0] = tmp[0];
}

void InvertUint16(uint16_t *dBuf, uint16_t *srcBuf)
{
	int i;
	uint16_t tmp[4];
	tmp[0] = 0;
	for (i = 0; i < 16; i++)
	{
		if (srcBuf[0] & (1 << i))
			tmp[0] |= 1 << (15 - i);
	}
	dBuf[0] = tmp[0];
}

uint16_t CRC16_MODBUS(uint8_t *puchMsg, uint32_t usDataLen, uint16_t wCRCin)
{
	//uint16_t wCRCin = 0xFFFF;
	uint16_t wCPoly = 0x8005;
	uint8_t wChar = 0;
	while (usDataLen--)
	{
		wChar = *(puchMsg++);
		InvertUint8(&wChar, &wChar);
		wCRCin ^= (wChar << 8);
		for (int i = 0; i < 8; i++)
		{
			if (wCRCin & 0x8000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	InvertUint16(&wCRCin, &wCRCin);
	return (wCRCin);
}

void MSR_MSP(uint32_t addr)
{
	asm volatile
	(
			"MSR MSP, r0 \n"
			"BX r14 \n"
	);
}

//void initWriteHandle(writeHandle_t wHandle)
//{
//  wHandle.writeOffsetAddr = 0;
//  wHandle.flashAppAddr = FLASH_APPBACKUP_ADDR;
//}

//get one block app bin data ,and use this store in flash memery
Iap_ErrorCode iap_temporaryStore_appbin()
{

	uint16_t i = 0;
	uint16_t curBlockIndex = 0;
	uint16_t crc = 0;
	uint32_t flashStartBaseAddr = 0;

	//first of all ,check appbin crc,if in appbin has already do this step in receive,it can
	//be skip this step!
	if (binDataCrc == 0)
	{
		binDataCrc = bd.AppbinCrc;
	}
	else
	{
		if (binDataCrc != bd.AppbinCrc)
		{
			return BIN_CRC_ERROR;
		}
	}

	if (binDataTotalSize == 0)
	{
		binDataTotalSize = bd.AppbinToltalSize;
	}
	else
	{
		if (binDataTotalSize != bd.AppbinToltalSize)
		{
			return BIN_TOTAL_SIZE_ERROR;
		}
	}

	flashStartBaseAddr = FLASH_START_ADDR_APP2
	;

	curBlockIndex = bd.AppbinCurBlockIndex - 1;
	fwaddr = flashStartBaseAddr + curBlockIndex * 512;

	if (CRC16_MODBUS(bd.AppbinDataBlock, 512, 0xffff) == bd.AppbinDataBlockCrc)
	{
		//crc successful
		stmflash_write(fwaddr, (uint16_t*) &bd.AppbinDataBlock[0], 512 / 2);

	}
	else
	{
		return BLOCK_CRC_ERROR;
	}

	if ((fwaddr + 512 - flashStartBaseAddr) >= binDataTotalSize)
	{

		fwaddr = flashStartBaseAddr;
		//crc calcalate for whole bin file
		for (i = 0; i < bd.AppbinCurBlockIndex; i++)
		{

			stmflash_read_bytes(fwaddr, iapbuf, 256);

//			if(i != bd.AppbinCurBlockIndex-1)
//			{
//				STMFLASH_Read(fwaddr,iapbuf,256);
//			}
//			else
//			{
//				STMFLASH_Read(fwaddr,iapbuf,binDataTotalSize - 512*(curBlockIndex));
//			}
			//crc 累计计算，第一个block是需要初始化计算，后续可以用累计计算
			if (i == 0)
			{
				crc = CRC16_MODBUS((uint8_t*) &iapbuf[0], 512, 0xFFFF);
			}
			else
			{
				crc = CRC16_MODBUS((uint8_t*) &iapbuf[0], 512, crc);
			}
			fwaddr += 512;
		}

		if (crc != bd.AppbinCrc)
		{
			return BIN_CRC_ERROR;
		}
		//write appbinHandle.flag ,in order to next reset mcu run this app
		//if((ufpt.HappBin.flag & 0x80)!=0x80){
		//switch to update mode
		appBin.flag = 0xaa;
		appBin.appBinByteSize = bd.AppbinToltalSize;
		appBin.appBinCrc = bd.AppbinCrc;
		appBin.srcFlashAddr = FLASH_START_ADDR_APP2;

		//保存到内部flash
		stmflash_write(STMFLASH_BASE + STMFLASH_IAP_INFO_BASE,
								(uint16_t*) &appBin, sizeof(appBin) / 2);
		return BIN_FILE_DOWN_OK;
	}
	return BIN_BLOCK_DOWN_OK;
}

void iap_write_appbin(uint32_t appxaddr, uint32_t appbufaddr, uint16_t appsize)
{
	uint16_t t;
	uint16_t i = 0;
	uint32_t fwaddr = appxaddr;

	//first of all ,check appbin crc,if in appbin has already do this step in receive,it can
	//be skip this step!

	//write app bin to first app flash address

	for (t = 0; t < appsize; t += 2048)
	{
		stmflash_read_bytes(appbufaddr, iapbuf, 1024);
		stmflash_write(fwaddr, iapbuf, 1024);
		fwaddr += 2048;
		appbufaddr += 2048;
		i++;
	}
	//write rest bytes
	if ((appsize - i * 2048) > 0)
	{
		stmflash_read_bytes(appbufaddr, iapbuf, (appsize - i * 2048) / 2);
		stmflash_write(fwaddr, iapbuf, (appsize - i * 2048) / 2);
	}
}

void iap_load_app(uint32_t appxaddr)
{
	if (((*(vu32*) appxaddr) & 0x2FFE0000) == 0x20000000)
	{
		jump2app = (iapfun) *(vu32*) (appxaddr + 4);
		MSR_MSP(*(vu32*) appxaddr);
		jump2app();
	}
}
