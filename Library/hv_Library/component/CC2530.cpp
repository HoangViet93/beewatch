/**
  ******************************************************************************
 * @file    CC2530.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    11-5-2015
 * @brief   CC2530 Zigbee Network Processor driver
  */
//-------------------------------------------------------------------------
#include "CC2530.h"

namespace hv_driver {

CC2530::CC2530(GPIO* rstPin, GPIO* srdyPin, GPIO* mrdyPin, SPI* spi,GPIO* ssPin){
	this->rstPin = rstPin;
	this->srdyPin = srdyPin;
	this->mrdyPin = mrdyPin;
	this->spi	= spi;
	this->ssPin = ssPin;
	
	this->retCMD = NONE;
	this->retLen = 0;
	this->isInit = false;
}	

bool CC2530::init(void){
	this->rstPin->initOutput(GPIO::PP, GPIO::MEDIUM);
	this->ssPin->initOutput(GPIO::PP, GPIO::MEDIUM);
	this->mrdyPin->initOutput(GPIO::PP, GPIO::MEDIUM);
	this->srdyPin->initInput(GPIO::NONE);
	
	this->spi->init(SPI::MASTER, SPI::BAUDRATE_DIV32, SPI::NSS_SOFT, SPI::CPHA_2EDGE, SPI::CPOL_HIGH);
	this->rstPin->reset();
	this->mrdyPin->set();
	this->ssPin->set();
	
	if(this->reset() != true){
		return false;
	}
	this->isInit = true;
	return true;
}

void CC2530::Poll(void){
	uint8_t temp[3] = {0, 0, 0};
	
	while(this->srdyPin->read() == 1);
	this->mrdyPin->reset();
	this->ssPin->reset();
	
	this->spi->transmit(temp, 3);
	while(this->srdyPin->read() == 0);
	
	this->spi->receive(temp, 3);
	this->spi->receive(this->retData, temp[0]);
	this->retCMD = (COMMAND)((uint16_t)(temp[1] << 8) + (uint16_t)(temp[2]));
	this->retLen = temp[0];
	
	this->mrdyPin->set();
	this->ssPin->set();
}

bool CC2530::SREQ(uint16_t cmd, uint8_t *txPtr, uint8_t len){
	uint8_t temp[3] = {0, 0, 0};
	
	if((txPtr == NULL && len != 0) || (txPtr != NULL && len == 0)){
		return false;
	}
	this->mrdyPin->reset();
	this->ssPin->reset();
	while(this->srdyPin->read() == 1);
	
	temp[0] = len;
	temp[1] = (uint8_t)(cmd >> 8);
	temp[2] = (uint8_t)(cmd);
	this->spi->transmit(temp, 3);
	this->spi->transmit(txPtr, len);
	while(this->srdyPin->read() == 0);
	
	this->spi->receive(temp, 3);
	this->spi->receive(this->retData, temp[0]);
	this->retCMD = (COMMAND)((uint16_t)(temp[1] << 8) + (uint16_t)(temp[2]));
	this->retLen = temp[0];
	
	this->mrdyPin->set();
	this->ssPin->set();
	return true;
}

bool CC2530::reset(void){
	this->rstPin->reset();
	HAL_Delay(2);
	this->rstPin->set();
	HAL_Delay(5);
	
	this->Poll();
	if(this->retCMD != SYS_RESET_IND){
		return false;
	}
	
	/*	get chip release ID	*/
	this->revID.resetReason =  retData[0];
	this->revID.transportRev = retData[1];
	this->revID.productID = 	 retData[2];
	this->revID.releaseNum = 	 (uint16_t)(retData[4] << 8) + (uint16_t)retData[3];
	this->revID.hwRev = 			 retData[5];

	return true;
}

bool CC2530::checkNewMes(void){
	if(this->srdyPin->read() != 0){
		return false;
	}
	this->Poll();
	return true;
}

} /* hv_driver namespace */
