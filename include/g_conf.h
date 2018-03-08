/*
 * g_conf.h
 *
 *  Created on: 2017年11月28日
 *      Author: hxbao
 */

#ifndef INCLUDE_G_CONF_H_
#define INCLUDE_G_CONF_H_

#include "eeprom.h"
#define DEBUG 1

enum
{
	INTER_EXTL0= 8,
	INTER_UART1 =9,
	INTER_UART2 =10,
	INTER_UART5 =11,
	INTER_TIM2 =12,
	INTER_TIM3 = 13,
	INTER_EXTL3 = 14

};

#define FW_VERSION 1
//需要eeprom 保持的对象结构
//参数对象1
typedef struct
{
	uint32_t devid;  //设备的序列号
	uint8_t hwver;	 //硬件版本号
	uint8_t fwver;   //固件版本号
	char  alias[32]; //设备的别名
	//uint8_t c;
}Dev_Info_Def;

//参数结构对象2
typedef struct
{
	uint8_t a;
	uint8_t b;
	uint8_t c;
}EEPROM_PARA_2_Def;

#define EE_DEV_INFO_ADDR (EEPROM_PARA1_BASE)

#if 1//CDDZ

#define ONENET_PRODUCT_ID "112286"
#define ONENET_MKEY "mTebuQmvziqUi0rWFjz8GsC77L4="
#define ONENET_DESP "cddz"

#endif

#if 0 //ZQDZ
#define ONENET_PRODUCT_ID "97821"
#define ONENET_MKEY "UjEKcwTj0FCil3dmwekpUGX4eHY="
#define ONENET_DESP "zqdz"
#endif

//太湖能谷key
//#define ONENET_PRODUCT_ID "82709"
//#define ONENET_MKEY "dxuZJFvgGSSzDLxUXw06NXupPG0="
//#define ONENET_DESP "thng"



#endif /* INCLUDE_G_CONF_H_ */
