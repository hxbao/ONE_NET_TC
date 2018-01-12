/*
 * mymemery.h
 *
 *  Created on: 2017Äê12ÔÂ1ÈÕ
 *      Author: hxbao
 */

#ifndef INCLUDE_MYMEMERY_H_
#define INCLUDE_MYMEMERY_H_
#include "stm32f10x.h"

int16_t my_mem_percentage_used();
uint8_t * my_malloc(uint32_t size);
uint8_t my_free(uint8_t* ptr);

#endif /* INCLUDE_MYMEMERY_H_ */
