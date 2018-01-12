/*
 * misc.h
 *
 *  Created on: 2018Äê1ÔÂ9ÈÕ
 *      Author: hxbao
 */

#ifndef INCLUDE_MYMISC_H_
#define INCLUDE_MYMISC_H_
#include "stdint.h"
int8_t asc2hex3(char *a3,uint16_t len,uint8_t *pOutBuffer);
int8_t hex2ascString(uint8_t *psrc,char *pdes,uint16_t len);
void sys_stop_delay(uint32_t ms);

#endif /* INCLUDE_MYMISC_H_ */
