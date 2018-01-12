/*
 * iwdg.c
 *
 *  Created on: 2018Äê1ÔÂ11ÈÕ
 *      Author: hxbao
 */

#include "stm32f10x.h"

void iwdg_init()
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(4095);
	IWDG_Enable();
}

void iwdg_kick()
{
	IWDG_ReloadCounter();
}
