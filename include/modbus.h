/*
 * modbus.h
 *
 *  Created on: 2018Äê1ÔÂ9ÈÕ
 *      Author: hxbao
 */

#ifndef INCLUDE_MODBUS_H_
#define INCLUDE_MODBUS_H_



#define TEMP_BUF_SIZE 256
extern uint8_t modbus_temp_buf[TEMP_BUF_SIZE];
extern uint8_t modbus_temp_len ;
extern uint8_t IsWrite485Buffer;
extern uint8_t modbus_readBuf[256];
extern uint8_t modbus_writeBuf[256];

extern uint8_t readBufCnt;
extern uint8_t writeBufCnt;

extern uint8_t lasttimeSendData2OnenetStu ;
extern uint8_t stu_count ;

void modbus_init();
void modbus_task();

#endif /* INCLUDE_MODBUS_H_ */
