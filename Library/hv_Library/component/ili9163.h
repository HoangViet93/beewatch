/**
  ******************************************************************************
 * @file    ili9163.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    20-11-2015
 * @brief   ili9163 TFT driver
  */
//-------------------------------------------------------------------------

#ifndef ILI9163_H
#define ILI9163_H

#include "stm32f1xx.h"
#include "GPIO.h"
#include "SPI.h"
#include "Graphic.h"
#include "RTC.h"

namespace hv_driver {

class ILI9163 {
public:
	ILI9163(SPI* spi, GPIO* csPin, GPIO* A0Pin, GPIO* rstPin, GPIO* BLPin);

	void init(void);

	void setAddress(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
	void setScreen(uint16_t color);
	void invertMode(bool isInvert);
	void writePix(uint8_t x, uint8_t y, uint16_t color);

	bool putChar(uint8_t x, uint8_t y, char ch, font_s &font);
	bool putStr(uint8_t x, uint8_t y, const char* str, font_s &font);
	bool drawBitmap(uint8_t x, uint8_t y, bitMap_s &bitMap);
	bool drawPicture(uint8_t x, uint8_t y, picture_s &picture);
	void putClock(uint8_t x, uint8_t y, _RTC::time_s &time, font_s &font);
	//void drawLine(uint8_t x, uint8_t y, bool isStraightLine, uint8_t thinkness, uint16_t color);

	void sendByte(uint8_t data);
	void sendWord(uint16_t data);
	void sendCMD(uint8_t cmd);
private:
	SPI* spi;
  GPIO* csPin;
	GPIO* A0Pin;
	GPIO* rstPin;
	GPIO* BLPin;
};	

}

#endif /* ILI9163_H */