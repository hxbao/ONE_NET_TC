/*
 * test.h
 *
 *  Created on: 2017Äê12ÔÂ1ÈÕ
 *      Author: hxbao
 */

#ifndef INCLUDE_TEST_H_
#define INCLUDE_TEST_H_
#include "stm32f10x.h"
#include "i2c.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "mymemery.h"
#include "commd_if.h"
#include "diag/trace.h"
#include "eeprom.h"

void test_uart1();
void test_uart2();
void test_uart5();
void test_spi_master();
void test_spi_slave();
void test_timer();
void test_mymemery();
void test_eeprom();




#endif /* INCLUDE_TEST_H_ */
