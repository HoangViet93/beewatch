/**
  ******************************************************************************
 * @file    SPI.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    20-11-2015
 * @brief   SPI driver
  */
//-------------------------------------------------------------------------
#include "SPI.h"

namespace hv_driver {

SPI::SPI(SPI_TypeDef* SPIx){
	this->SPIx = SPIx;
	this->mode = MASTER;
}	

void SPI::init(MODE mode, BAUD_DIV baud, NSS nss, CPHA cpha, CPOL cpol){
	uint32_t temp;
	this->mode = mode; // update data member 
	
	this->SPIx->CR1 &= ~SPI_CR1_SPE; // disable SPI first
	
	/* write config to SPI control register */
	temp = this->SPIx->CR1;
	temp &= ~SPI_CR1_BIDIMODE; // disable bidirection mode
	temp &= ~SPI_CR1_BIDIOE; // disable bidirection output;
	temp &= ~SPI_CR1_CRCEN; // disable CRC mode
	temp &= ~SPI_CR1_CRCNEXT; // disable CRC next
	temp &= ~SPI_CR1_DFF; // data frame 8bit
	temp &= ~SPI_CR1_RXONLY; // full duplex mode by disable rx only
	temp |= (nss & SPI_CR1_SSM); // slave select pin mode
	if(mode == MASTER){
		temp |= SPI_CR1_SSI; // if master mode is set SSI bit is assign into the slave select pin, value of ss pin is ignore
	} else {
		temp &= ~SPI_CR1_SSI; // slave mode reset bit
	}
	temp &= ~SPI_CR1_LSBFIRST; // clear LSBFIRST bit, msb first
	temp |= baud << 3; // write 3 bit baudrate
	temp |= mode; // write mode 
	temp |= cpol; // write clock polarity mode
	temp |= cpha;  // write clock phase mode
	
	this->SPIx->CR1 = temp;
	this->SPIx->CR2 = ((nss >> 16) & SPI_CR2_SSOE); // disable or enable ss pin output following nss pin mode
	//this->SPIx->CR2 &= ~SPI_CR2_FRF; // disable TI mode
	this->SPIx->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD; // disable I2S mode, enable spi mode

	this->SPIx->CR1 |= SPI_CR1_SPE; // enable spi
}

uint8_t SPI::tranceiverByte(uint8_t ch){
	while((this->SPIx->SR & SPI_SR_TXE) != SPI_SR_TXE); // wait txe flag is set txbuffer data transfer to shift register
	this->SPIx->DR = ch;
	while((this->SPIx->SR & SPI_SR_RXNE) != SPI_SR_RXNE); // wait rxne flag is set data is on the rx buffer
	return this->SPIx->DR; // read rxBuffer to clear the rxne flag
}

void SPI::transceiver(uint8_t *txPtr, uint8_t *rxPtr, uint8_t size){
	if((this->SPIx->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){ // if spi not enable
		this->SPIx->CR1 |= SPI_CR1_SPE; // enable spi
	}
	for(uint8_t i = 0; i < size; i++){
		rxPtr[i] = this->tranceiverByte(txPtr[i]);
	}
}

void SPI::transmit(uint8_t *txPtr, uint8_t size){
	uint8_t temp;
	if((this->SPIx->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){ // if spi not enable
		this->SPIx->CR1 |= SPI_CR1_SPE; // enable spi
	}	
	for(uint8_t i = 0; i < size; i++){
		temp = this->tranceiverByte(txPtr[i]);
	}
}

void SPI::receive(uint8_t *rxPtr, uint8_t size){	
	if((this->SPIx->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){ // if spi not enable
		this->SPIx->CR1 |= SPI_CR1_SPE; // enable spi
	}	

	if(this->mode == MASTER){
		/*	is master mode send dummy byte to generate clock on mosi */
		for(uint8_t i = 0; i < size; i++){
			rxPtr[i] = this->tranceiverByte(0xFF);
		}
	} else {
		for(uint8_t i = 0; i < size; i++){
			while((this->SPIx->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
			rxPtr[i] = this->SPIx->DR;
		}		
	}
}

} /* namespace hv_driver */