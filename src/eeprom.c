/*
 * eeprom.c
 *
 *  Created on: 2017Äê12ÔÂ15ÈÕ
 *      Author: hxbao
 */
#include "stm32f10x.h"
#include "eeprom.h"

typedef struct
{
	__IO uint32_t ACR1;
	__IO uint32_t PECR1;
	__IO uint32_t PDKEYR1;
	__IO uint32_t PEKEYR1;
	__IO uint32_t PRGKEYR1;
	__IO uint32_t OPTKEYR1;
	__IO uint32_t SR1;
	__IO uint32_t OBR1;
	__IO uint32_t WRPR1;

} MY_FLASH_TypeDef;



#define MYFLASH               ((MY_FLASH_TypeDef *) FLASH_R_BASE)

void EEPROM_ReadBytes(uint16_t Addr, uint8_t *Buffer, uint16_t Length)
{
	uint8_t *wAddr;
	wAddr = (uint8_t*) (EEPROM_BASE_ADDR + Addr);
	while (Length--)
	{
		*Buffer++ = *wAddr++;
	}
}

void EEPROM_ReadWords(uint16_t Addr, uint16_t *Buffer, uint16_t Length)
{
	uint32_t *wAddr;
	wAddr = (uint32_t*) (EEPROM_BASE_ADDR + Addr);
	while (Length--)
	{
		*Buffer++ = *wAddr++;
	}
}

#define PEKEY1 0x89ABCDEF//FLASH_PEKEYR
#define PEKEY2 0x02030405//FLASH_PEKEYR
#define FLASH_PECR_PELOCK   ((uint32_t)0x00000001)
#define FLASH_PECR_FTDW 	((uint32_t)0x00000100)
#define FLASH_SR_BSY1 		((uint32_t)0x00000001)

#define DIS_INT()  __disable_irq()
#define EN_INT()	__enable_irq();

void EEPROM_WriteBytes(uint16_t Addr, uint8_t *Buffer, uint16_t Length)
{
	uint8_t *wAddr;
	wAddr = (uint8_t *) (EEPROM_BASE_ADDR + Addr);
	DIS_INT();
	MYFLASH->PEKEYR1 = PEKEY1;
	MYFLASH->PEKEYR1 = PEKEY2;
	while (MYFLASH->PECR1 & FLASH_PECR_PELOCK)
		;
	MYFLASH->PECR1 |= FLASH_PECR_FTDW; //not fast write
	while (Length--)
	{
		*wAddr++ = *Buffer++;
		while (MYFLASH->SR1 & FLASH_SR_BSY1)
			;
	}
	MYFLASH->PECR1 |= FLASH_PECR_PELOCK;
	EN_INT();
}

