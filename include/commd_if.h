/*
 * commd_if.h
 *
 *  Created on: 2017年11月28日
 *      Author: hxbao
 */

#ifndef INCLUDE_COMMD_IF_H_
#define INCLUDE_COMMD_IF_H_
#include "stm32f10x.h"
#include "modbus.h"
#include "mymisc.h"

#define COMMD_BUF_SIZE 1024

#define commd_send_ack() rola_send_data((uint8_t*)"CMD_ACK_OK", strlen("CMD_ACK_OK"), 1)

//接收的命令缓冲
extern uint8_t commd_buf[COMMD_BUF_SIZE];
//当前 接收的命令的长度
extern uint16_t commd_cur_len;
uint8_t diableCommdPoll;

//#define COMMD_START_SAMPLE "AT+CMDGET_SAMPLE_START="
//#define COMMD_STOP_SAMPLE "AT+CMDSET_STOP_SAMPLE"
//#define COMMD_CONFIG_HZ "AT+CMDSET_SAMPLE_HZ="
//#define COMMD_CONFIG_ROLA_ADDR "AT+CMDSET_ROLA_ADDR="
//#define COMMD_STOP_ROLA_SEND "AT+CMDSET_STOP_ROLA_SEND"

void poll_store_commd();
uint8_t get_commd();
//void commd_poll();

void load_cmd_para_from_flash();
void store_cmd_para_to_flash_task();
void task_todo();



#endif /* INCLUDE_COMMD_IF_H_ */
