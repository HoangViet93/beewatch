/**
  ******************************************************************************
 * @file    ili9163.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    20-11-2015
 * @brief   ili9163 TFT driver
  */
//-------------------------------------------------------------------------
#include "ili9163.h"
#include "string.h"
#include "MISC.h"

namespace hv_driver {

ILI9163::ILI9163(SPI* spi, GPIO* csPin, GPIO* A0Pin, GPIO* rstPin, GPIO* BLPin){
	this->spi = spi;
	this->csPin = csPin;
	this->A0Pin = A0Pin;
	this->rstPin = rstPin;
	this->BLPin = BLPin;
}

void ILI9163::init(void){
	this->csPin->initOutput(GPIO::PP, GPIO::MEDIUM);
	this->A0Pin->initOutput(GPIO::PP, GPIO::MEDIUM);
	this->rstPin->initOutput(GPIO::PP, GPIO::MEDIUM);
	this->BLPin->initOutput(GPIO::PP, GPIO::MEDIUM);
	
	__HAL_RCC_SPI2_CLK_ENABLE();
	this->spi->init(SPI::MASTER, SPI::BAUDRATE_DIV2, SPI::NSS_SOFT, SPI::CPHA_1EDGE, SPI::CPOL_LOW);
	
	this->rstPin->reset();
	Sys_Delayms(20);
  this->rstPin->set();
	Sys_Delayms(20);
  this->csPin->reset();	
	
	this->sendCMD(0x01);
	this->sendCMD(0x11);
	Sys_Delayms(20);
	
	this->sendCMD(0x26);
	this->sendByte(0x04);
	
	this->sendCMD(0xC0);
	this->sendByte(0x1F);

	this->sendCMD(0xC1);
	this->sendByte(0x00);

	this->sendCMD(0xC2);
	this->sendByte(0x00);
	this->sendByte(0x07);
	
	this->sendCMD(0xC3);
	this->sendByte(0x00);
	this->sendByte(0x07);

	this->sendCMD(0xC5);
	this->sendByte(0x24);
	this->sendByte(0xC8);

	this->sendCMD(0x38);

	this->sendCMD(0x3A);
	this->sendByte(0x05);

	this->sendCMD(0x36);
	this->sendByte(0x08);
	
	this->sendCMD(0x29);

	this->invertMode(false);
	//this->setScreen(0xFFFF);
	this->BLPin->set();
}

void ILI9163::setAddress(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
	this->sendCMD(0x2A);
	this->sendByte(0x00);
	this->sendByte(x1);
	this->sendByte(0x00);
	this->sendByte(x2);
	
	this->sendCMD(0x2B);
	this->sendByte(0x00);
	this->sendByte(32 + y1);
	this->sendByte(0x00);
	this->sendByte(32 + y2);
	
	this->sendCMD(0x2C);
}

void ILI9163::setScreen(uint16_t color){
	this->setAddress(0, 0, 127, 127);
	for(uint8_t rowCount = 0; rowCount < 128; rowCount++){
		for(uint8_t colCount = 0; colCount < 128; colCount++){
			this->sendWord(color);
		}
	}
}

void ILI9163::writePix(uint8_t x, uint8_t y, uint16_t color){
	this->setAddress(x, y, x, y);
	this->sendWord(color);
}

void ILI9163::invertMode(bool isInvert){
	if(isInvert == true){
		this->sendCMD(0x21);
	} else {
		this->sendCMD(0x20);
	}
}

bool ILI9163::putChar(uint8_t x, uint8_t y, char ch, font_s &font){
	uint16_t charWidth,test,temp;
	uint16_t startChar, byteHeight;
	
	byteHeight = (font.height % 8) > 0 ? (font.height / 8 + 1) : (font.height / 8);
	
	charWidth = font.fontCode[(ch - 32) * (font.width * byteHeight + 1)];
	startChar = (ch - 32) * (font.width * byteHeight + 1) + 1;
	
	this->setAddress(x, y, x + charWidth - 1, y + font.height - 1);
	
	for(uint16_t yCount = 0; yCount < font.height;){
		
		for(uint16_t xCount = 0; xCount < charWidth; xCount++){
			if((font.fontCode[startChar + (byteHeight * xCount)] >> (yCount - 8 * temp)) & 1){
				//test = 1;
				this->sendWord(font.textColor);
			} else {
				//test = 0;
				this->sendWord(font.bkgColor);
			}
		}
		yCount++;
		temp = yCount / 8;
		if(yCount == 8 * temp){
			startChar++;
		}
	}
}

bool ILI9163::putStr(uint8_t x, uint8_t y, const char* str, font_s &font){
	uint8_t len = strlen(str);
	uint8_t byteHeight;
	uint16_t x1,y1;
	x1 = y1 = 0;
	
	byteHeight = (font.height % 8) > 0 ? (font.height / 8 + 1) : (font.height / 8);
	for(uint8_t i = 0; i < len; i++){
		this->putChar(x + x1, y, str[i],font);
		x1 += font.fontCode[(str[i] - 32) * (font.width * byteHeight + 1)];
		if((x + x1 + (font.fontCode[(str[i + 1] - 32) * (font.width * byteHeight + 1)])) >= 127){
			y+= font.height;
			x1 = 0;
		}
	}
}

bool ILI9163::drawBitmap(uint8_t x, uint8_t y, bitMap_s &bitMap){
	uint8_t test;
	this->setAddress(x, y, x + bitMap.width - 1, y + bitMap.height - 1);
	for(uint16_t yCount = 0; yCount < bitMap.height; yCount++){
		for(uint16_t xCount = 0; xCount < (bitMap.width / 8); xCount++){
			for(int8_t i = 7;i >= 0; i--){
				if((bitMap.code[xCount + yCount*(bitMap.width / 8)] >> i) & 1){
					this->sendWord(bitMap.bitmapColor);
				} else {
					this->sendWord(bitMap.bkgColor);
				}
			}
		}
	}
}

bool ILI9163::drawPicture(uint8_t x, uint8_t y, picture_s &picture){
	this->setAddress(x, y, x + picture.width - 1, y + picture.height - 1);;
	for(uint16_t i = 0; i < picture.width * picture.height; i++){
		this->sendWord(picture.code[i]);
	}
}

void ILI9163::putClock(uint8_t x, uint8_t y, _RTC::time_s &time, font_s &font){
	uint8_t timeData[10];
	sprintf((char*)timeData, "%0.2d:%0.2d:%0.2d", time.hours, time.minutes, time.seconds);
	this->putStr(x, y, (const char*)timeData, font);
}

void ILI9163::sendCMD(uint8_t cmd){
	this->A0Pin->reset();
	//this->csPin->reset();	
	//while((SPI1->SR & SPI_FLAG_TXE) == 0);
	//SPI2->DR = cmd;
	//while((SPI1->SR & SPI_FLAG_RXNE) == 0);
	//while((SPI2->SR & SPI_FLAG_BSY) == 1);
	//this->csPin->set();	
	this->spi->tranceiverByte(cmd);
}

void ILI9163::sendByte(uint8_t data){
	this->A0Pin->set();
	//this->csPin->reset();	
	//while((SPI1->SR & SPI_FLAG_TXE) == 0);
	//SPI2->DR = data;
	//while((SPI1->SR & SPI_FLAG_RXNE) == 0);
	//while((SPI2->SR & SPI_FLAG_BSY) == 1);
	//this->csPin->set();
	this->spi->tranceiverByte(data);
}

void ILI9163::sendWord(uint16_t data){
	this->A0Pin->set();
	this->spi->tranceiverByte(data >> 8);
	this->spi->tranceiverByte(data);
}

}