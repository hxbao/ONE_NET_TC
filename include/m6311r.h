/*
 * m6311r.h
 *
 *  Created on: 2018Äê1ÔÂ9ÈÕ
 *      Author: hxbao
 */

#ifndef INCLUDE_M6311R_H_
#define INCLUDE_M6311R_H_
#include "stdint.h"

extern uint8_t one_connect_state ;
extern uint8_t reM6311Start;

void gprs_connect();
void onenet_init();
uint8_t iot_onenet_send_ping();
uint8_t iot_onenet_send_bin_data(uint8_t *databuf, uint16_t dataLen);
void iot_send_simcard_id(void);
void iot_send_csq(uint32_t count);
uint8_t iot_onenet_read(uint8_t *pRecvBuffer);
void iot_onenet_task(uint32_t count);
void m6311r_reset();
void iot_send_iap_info(char *iapinfo);

#endif /* INCLUDE_M6311R_H_ */
