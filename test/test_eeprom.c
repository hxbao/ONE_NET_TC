/*
 * test_eeprom.c
 *
 *  Created on: 2017Äê12ÔÂ15ÈÕ
 *      Author: hxbao
 */

#include "test.h"
#include "string.h"

void test_eeprom()
{
	Dev_Info_Def di;
	di.devid = 0x12345678;
	di.hwver = 1;
	di.fwver = 1;
	strcpy(di.alias,"sumyong rola 1");
	EEPROM_WriteBytes(EE_DEV_INFO_ADDR, (uint8_t*) &di, sizeof(Dev_Info_Def));

	di.devid = 0;
	di.hwver = 0;
	di.fwver = 0;
	*(di.alias) = 0;
	EEPROM_ReadBytes(EE_DEV_INFO_ADDR, (uint8_t*) &di, sizeof(Dev_Info_Def));

	trace_printf(
			"eeprom store device info: devid:%d,hwver:%d,fwver:%d,alias:%s\n",
			di.devid, di.fwver, di.fwver, di.alias);
	while (1)
		;
}
