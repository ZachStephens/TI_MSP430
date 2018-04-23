/*
 * I2C.h
 *
 *  Created on: Mar 21, 2018
 *      Author: quant
 */

#ifndef I2C_H_
#define I2C_H_

#include "msp430g2553.h"
#include <stdint.h>

void I2C_Init();

uint8_t I2C_WriteByte(uint8_t, uint8_t, uint8_t);

uint8_t I2C_ReadByte(uint8_t, uint8_t);

uint8_t I2C_WriteData(uint8_t, uint8_t, uint8_t, uint8_t);

uint8_t I2C_ReadData(uint8_t*, uint8_t, uint8_t , uint8_t );




#endif /* I2C_H_ */
