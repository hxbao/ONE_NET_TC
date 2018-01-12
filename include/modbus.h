/*
 * modbus.h
 *
 *  Created on: 2018��1��9��
 *      Author: hxbao
 */

#ifndef INCLUDE_MODBUS_H_
#define INCLUDE_MODBUS_H_

extern uint8_t IsWrite485Buffer;
extern uint8_t modbus_readBuf[256];
extern uint8_t modbus_writeBuf[256];

extern uint8_t readBufCnt;
extern uint8_t writeBufCnt;

extern uint8_t lasttimeSendData2OnenetStu ;
extern uint8_t stu_count ;

void modbus_task();

#endif /* INCLUDE_MODBUS_H_ */
