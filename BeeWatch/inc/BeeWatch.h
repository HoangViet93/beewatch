/**
  ******************************************************************************
 * @file    BeeWatch.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    6-12-2015
 * @brief   BeeWatch main source file
  */
//-------------------------------------------------------------------------

#ifndef BEEWATCH_H
#define BEEWATCH_H

#include "stm32f1xx.h"
#include "Graphic.h"
#include "GPIO.h"
#include "ILI9163.h"
#include "ADXL345.h"
#include "CC2530.h"
#include "Z_stack.h"
#include "HeartRate.h"

namespace hv_driver {

class BeeWatch {
public:
	enum BATTERY_LEVEL {
		LOW, MEDIUM, FULL, CHARGING
	};
	enum ACTIVITY_STATUS {
		ACTIVITY, FREE_FALL, INACTIVITY
	};
	enum ZB_COMMAND {
		STATUS = 0xABCD, FALL_ALERT = 0xABCE, HEART_RATE = 0xABCF
	};
public:
	BeeWatch(void);
	void init(void);
	void initGyro(void);
	void initZigbee(void);

	void sendAlert(void);
	void sendMessage(void);
	void sendPPM(void);

	void checkGyroStatus(void);
	void drawBattery(uint8_t x, uint8_t y, BATTERY_LEVEL batLevel);
	void drawHeart(uint8_t x, uint8_t y);
	void updateHeartRate(uint8_t x, uint8_t y);
	void updateTime(uint8_t x, uint8_t y);
	void updateStatus(uint8_t x, uint8_t y);
	void updateNetWork(uint8_t x, uint8_t y, bool isConnected);
	void ActivitySttScreen(uint32_t actTime, uint32_t inActTime);
	
	ADXL345* getGyroInstant(void);
	ILI9163* getLCDInstant(void);
	font_s& getFont(void){return this->Bigfont;}
	font_s& getsmallFont(void){return this->smallFont;}

	ADXL345::IntVal_s& getStatus(void){return this->status;}
	
	uint32_t getActMin(void);
	uint32_t getInActMin(void);
private:
	bool isNewPPM;
	uint8_t heartRate;
	bitMap_s batteryBitmap;
	picture_s beePicture;
	font_s Bigfont;
	font_s smallFont;
	_RTC::time_s time;
	ADXL345::IntVal_s status;
	ADXL345::IntVal_s rawStatus;
	ACTIVITY_STATUS oldStatus;
};

}

#endif /* BEEWATCH_H */