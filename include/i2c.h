/*
 * i2c.h
 *
 *  Created on: 2017Äê11ÔÂ23ÈÕ
 *      Author: hxbao
 */

#ifndef I2C_H_
#define I2C_H_
#include "stm32f10x.h"

#define KX122A_I2C_INST I2C1
#define KX122B_I2C_INST I2C2

#define TIMEOUT 0x01
#define SUCESS  0x00

void i2c_hal_init(I2C_TypeDef* I2Cx);
uint8_t kx122_i2c_write_byte(I2C_TypeDef* I2Cx, uint8_t regAdd, uint8_t byte);
uint8_t kx122_i2c_read_byte(I2C_TypeDef* I2Cx, uint8_t regAdd,uint8_t *reRxData);
uint8_t kx122_i2c_read_Mbyte(I2C_TypeDef* I2Cx, uint8_t startRegAdd, uint8_t* pBuf,
		uint16_t num);
//void kx122_i2c_read_Mbyte_HS(I2C_TypeDef* I2Cx,uint8_t startRegAdd,uint8_t* pBuf);

#endif /* I2C_H_ */
