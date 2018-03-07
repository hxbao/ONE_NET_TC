/*
 * iap.h
 *
 *  Created on: 2018Äê3ÔÂ6ÈÕ
 *      Author: hxbao
 */

#ifndef INCLUDE_IAP_H_
#define INCLUDE_IAP_H_
#include "stm32f10x.h"

//#include "stmflash.h"

#define FLASH_START_ADDR_APP1 ADDR_FLASH_PAGE_5
#define FLASH_START_ADDR_APP2 0x8007FFF

//define jump function
typedef void (*iapfun)(void);        //

typedef enum
{
	BIN_CRC_ERROR,
	BLOCK_CRC_ERROR,
	BIN_TOTAL_SIZE_ERROR,
	BIN_FILE_DOWN_OK,
	BIN_BLOCK_DOWN_OK
} Iap_ErrorCode;

typedef struct
{
	uint16_t AppbinCurBlockIndex;
	uint16_t AppbinToltalSize;
	uint16_t AppbinCrc;
	uint8_t AppbinDataBlock[512]; //when the last block of appbin is not enough 512 bytes,
								  //fill the 0xff until the length is 512 bytes
	uint16_t AppbinDataBlockCrc;

} Iap_BlockData_def;

//define appHandle struct
typedef  struct
{
  uint8_t  flag;                      //bit7 update flag
  uint16_t appBinByteSize;
  uint32_t srcFlashAddr;
  uint32_t appBinCrc;
  char appbinVersion[6];

}AppBinHandle_t;

extern Iap_BlockData_def bd;
extern AppBinHandle_t appBin;

//define const flash address
#define FLASH_APP1_ADDR		      ADDR_FLASH_PAGE_5   //bootloader size : 16*256  = 4K bytes
#define FLASH_APPBACKUP_ADDR    ADDR_FLASH_PAGE_64  //the appbackup addr
#define FLASH_APP_HANDLE_ADDR   ADDR_FLASH_PAGE_127

Iap_ErrorCode iap_temporaryStore_appbin(void);
void iap_load_app(uint32_t appxaddr);			//
void iap_write_appbin(uint32_t appxaddr, uint32_t appbufaddr, uint16_t appsize);//write app bin at appxaddr

#endif /* INCLUDE_IAP_H_ */
