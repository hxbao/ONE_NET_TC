/*
 * stmflash.c
 *
 *  Created on: 2017��11��28��
 *      Author: hxbao
 */
#include "stm32f10x.h"
#include "mymemery.h"
#include "eeprom.h"

#define STM32_FLASH_BASE 0x08000000
#define STM_SECTOR_SIZE 1024 //�ֽ�
#define STM32_FLASH_SIZE 128

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void stmflash_read_bytes(uint32_t stmFlashAddr, uint16_t *pbuf,
		uint16_t size)
{
	uint16_t i;
	for (i = 0; i < size; i++)
	{
		pbuf[i] = *(__IO uint16_t*) stmFlashAddr; //��ȡ2���ֽ�.
		stmFlashAddr += 2; //ƫ��2���ֽ�.
	}
}

void stmflash_write_bytes_nocheck(uint32_t stmFlashAddr, uint16_t *pbuf, uint16_t size)
{
	uint16_t i;
	for (i = 0; i < size; i++)
	{
		FLASH_ProgramHalfWord(stmFlashAddr, *(pbuf + i));
		stmFlashAddr += 2; //��ַ����2.
	}
}

//WriteAddr: ��ʼ��ַ
//pBuffer:   ����ָ��
//NumToWrite:����(16λ)��
void stmflash_write(uint32_t stmFlashAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
	uint16_t i;

	uint32_t secpos;	   //������ַ
	uint16_t secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	uint16_t secremain;  //������ʣ���ַ(16λ�ּ���)
	uint32_t offaddr;    //ȥ��0X08000000��ĵ�ַ

	uint16_t *buf = (uint16_t*)my_malloc(STM_SECTOR_SIZE*2);

	if(buf ==0)
	{
		return;
	}

	if (stmFlashAddr < STM32_FLASH_BASE
			|| (stmFlashAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
		return;    //�Ƿ���ַ

	offaddr = stmFlashAddr - STM32_FLASH_BASE;		    //ʵ��ƫ�Ƶ�ַ.
	secpos = offaddr / STM_SECTOR_SIZE;			 //������ַ  0~127 for STM32F103RBT6
	secoff = (offaddr % STM_SECTOR_SIZE) / 2;		    //�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain = STM_SECTOR_SIZE / 2 - secoff;		        //����ʣ��ռ��С
	if (NumToWrite <= secremain)
		secremain = NumToWrite;  //�����ڸ�������Χ
	FLASH_Unlock();
	while (1)
	{
		stmflash_read_bytes(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, buf,
				STM_SECTOR_SIZE / 2);  //������������������

		for (i = 0; i < secremain; i++)  //У������
		{
			if (*(buf+secoff + i) != 0XFFFF)
				break;  //��Ҫ����
		}
		if (i < secremain)  //��Ҫ����
		{
			//����ҳ
			FLASH_ErasePage(secpos * STM_SECTOR_SIZE+ STM32_FLASH_BASE);

			for (i = 0; i < secremain; i++)  //����
			{
				*(buf+secoff + i) = pBuffer[i];
			}

			stmflash_write_bytes_nocheck(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE,
					buf, STM_SECTOR_SIZE / 2);  //д����������

		}
		else
			stmflash_write_bytes_nocheck(stmFlashAddr, pBuffer, secremain); //д�Ѿ������˵�,ֱ��д������ʣ������.
		if (NumToWrite == secremain)
			break;  //д�������
		else  //д��δ����
		{
			secpos++;				//������ַ��1
			secoff = 0;				//ƫ��λ��Ϊ0
			pBuffer += secremain;  	//ָ��ƫ��
			stmFlashAddr += secremain;	//д��ַƫ��
			NumToWrite -= secremain;	//�ֽ�(16λ)���ݼ�
			if (NumToWrite > (STM_SECTOR_SIZE / 2))
				secremain = STM_SECTOR_SIZE / 2;	//��һ����������д����
			else
				secremain = NumToWrite;	//��һ����������д����
		}
	}
	FLASH_Lock();
	my_free((uint8_t*)buf);

}

