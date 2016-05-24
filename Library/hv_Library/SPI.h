/**
  ******************************************************************************
 * @file    SPI.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    20-11-2015
 * @brief   SPI driver
  */
//-------------------------------------------------------------------------

#ifndef SPI_H
#define SPI_H

#include "stm32f1xx.h"

namespace hv_driver {
	
class SPI {
public:
	enum BAUD_DIV {
		BAUDRATE_DIV2 = 0, BAUDRATE_DIV4, BAUDRATE_DIV8, BAUDRATE_DIV16, BAUDRATE_DIV32, BAUDRATE_DIV64, BAUDRATE_DIV128, BAUDRATE_DIV256
	};
	enum MODE {
		SLAVE = 0x00, MASTER = 0x04
	};
	enum CPHA {
		CPHA_1EDGE = 0, CPHA_2EDGE = SPI_CR1_CPHA
	};
	enum CPOL {
		CPOL_LOW = 0, CPOL_HIGH = SPI_CR1_CPOL
	};
	enum NSS {
		NSS_SOFT = SPI_CR1_SSM, NSS_HARD_INPUT = 0x0000000, NSS_HARD_OUTPUT = 0x00040000
	};
public:	
	SPI(SPI_TypeDef* SPIx);
	
	void init(MODE mode, BAUD_DIV baud, NSS nss, CPHA cpha, CPOL cpol);

	uint8_t tranceiverByte(uint8_t ch);

	void transceiver(uint8_t *txPtr, uint8_t *rxPtr, uint8_t size);
	void transmit(uint8_t *txPtr, uint8_t size);
	void receive(uint8_t *rxPtr, uint8_t size);
private:
	SPI_TypeDef* SPIx;
	MODE mode;
};	
		
} /* hv_driver namespace */

#endif /*	SPI_H */