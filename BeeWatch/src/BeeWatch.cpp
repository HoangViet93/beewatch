/**
  ******************************************************************************
 * @file    BeeWatch.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    6-12-2015
 * @brief   BeeWatch main source file
  */
//-------------------------------------------------------------------------
#include "BeeWatch.h"

using namespace hv_driver;

GPIO PB7(GPIOB, GPIO::PIN7); 												
GPIO PB8(GPIOB, GPIO::PIN8);
GPIO PB9(GPIOB, GPIO::PIN9);
GPIO PA4(GPIOA, GPIO::PIN4);
GPIO PA5(GPIOA, GPIO::PIN5);
GPIO PA6(GPIOA, GPIO::PIN6);
GPIO PA7(GPIOA, GPIO::PIN7);

GPIO PB12(GPIOB, GPIO::PIN12); 
GPIO PB13(GPIOB, GPIO::PIN13);
GPIO PB14(GPIOB, GPIO::PIN14);
GPIO PB15(GPIOB, GPIO::PIN15);
GPIO PA8 (GPIOA, GPIO::PIN8);
GPIO PA9 (GPIOA, GPIO::PIN9);
GPIO PB1 (GPIOB, GPIO::PIN1);

SPI spi2(SPI2);
ILI9163 lcd(&spi2, &PB12, &PA9, &PA8, &PB14);
_RTC clock;
ADXL345 gyro(&PB1);

SPI spi1(SPI1);
CC2530 znp(&PB7, &PB9, &PB8, &spi1, &PA4);
Z_stack zigbee(&znp);
HeartRate ppm(ADC1, ADC_CHANNEL_0, GPIOA, GPIO_PIN_0);

uint32_t freeFallTime;

__IO uint32_t actCount = 0;
__IO uint32_t inActCount = 0;

__IO uint32_t actMin = 0;
__IO uint32_t inActMin = 0;
bool actFlag = false;

namespace hv_driver {

void FreeFallCount(void){
	freeFallTime--;
	if(freeFallTime == 0){
		Sys_subISRRemove(FreeFallCount);
	}
}

void ActivitySttCount(void){
	if(actFlag == true){
		actCount++;
		if(actCount >= 60000){
			actMin++; actCount = 0;
		}
	} else {
		inActCount++;
		if(inActCount >= 60000){
			inActMin++; inActCount = 0;
		}
	}	
}

void checkPPM(void){
	ppm.processing();
}

BeeWatch::BeeWatch(void){
	this->batteryBitmap.bitmapColor = WHITE;
	this->batteryBitmap.bkgColor = BLACK;
	this->batteryBitmap.width = 32;
	this->batteryBitmap.height = 16;
	this->time.hours = 0;
	this->time.minutes = 0;
	this->time.seconds = 0;
	this->heartRate = 0;
	this->isNewPPM = false;
}

void BeeWatch::init(void){

	PB13.initAF(GPIO::PP, GPIO::HIGH);
	PB15.initAF(GPIO::PP, GPIO::HIGH);
	
	lcd.init();
	lcd.setScreen(BLACK);
	clock.init();
	ppm.init();
	Sys_subISRAssign(checkPPM);
	
	Bigfont.height = 24;
	Bigfont.width = 24;
	Bigfont.bkgColor = BLACK;
	Bigfont.textColor = WHITE;
	Bigfont.fontCode = Arial24x24;
	
	smallFont.height = 16;
	smallFont.width = 15;
	smallFont.bkgColor = BLACK;
	smallFont.textColor = WHITE;
	smallFont.fontCode = Arial15x16;
}

void BeeWatch::initGyro(void){
	PB1.initInput(GPIO::DOWN);
	
	this->status.isActivity = false;
	this->status.isDataReady = false;
	this->status.isDoubleTap = false;
	this->status.isFreeFall = false;
	this->status.isInactivity = false;
	this->status.isOverrun = false;
	this->status.isWatermark = false;
	this->status.isTap = false;
	
	gyro.init(ADXL345::RANGE_16G, ADXL345::RATE_100HZ);
	gyro.setThreshActivity(2000); // 2g
	gyro.setActControl(ADXL345::AXIS_X, true);
	gyro.setActControl(ADXL345::AXIS_Y, true);
	gyro.setActControl(ADXL345::AXIS_Z, true);
	
	gyro.setThreshInactivity(185); // 0.185g
	gyro.setInactControl(ADXL345::AXIS_X, true);
	gyro.setInactControl(ADXL345::AXIS_Y, true);
	gyro.setInactControl(ADXL345::AXIS_Z, true);
	gyro.setInactivityTime(2); // 2s
	
	gyro.writeBit(ADXL345::ACT_INACT_CTL_REG, 3, true); // dc-couple ACT mode
	gyro.writeBit(ADXL345::ACT_INACT_CTL_REG, 7, false); // ac-couple InAct mode
	
	gyro.setThreshFreeFall(750); // 0.75g 
	gyro.setFreeFallTime(30); // 30ms
	
	gyro.useInterrupt(ADXL345::INT_PIN_1);
	
	Sys_subISRAssign(ActivitySttCount);
}

void BeeWatch::initZigbee(void){	
	__HAL_RCC_SPI1_CLK_ENABLE();
	
	PA5.initAF(GPIO::PP, GPIO::HIGH);
	PA6.initAF(GPIO::PP, GPIO::HIGH);
	PA7.initAF(GPIO::PP, GPIO::HIGH);
	
	zigbee.init(Z_stack::ZCD_startOpt_noClear, Z_stack::ZCD_endDevice, 0xFFFF);
	
	Z_stack::AppReg_s EndApp;
	EndApp.appEndPoint = 1;
	EndApp.appProfileID = 0x000A;
	EndApp.deviceID = 0x0001;
	EndApp.deviceVersion = 0x01;
	EndApp.inputCmdNum = 0;
	EndApp.outputCmd[0] = STATUS;
	EndApp.outputCmd[1] = FALL_ALERT;
	EndApp.outputCmd[2] = HEART_RATE;
	//EndApp.inputCmd[0] = 0xABCE;
	EndApp.outputCmdNum = 3;
	
	lcd.putStr(10, 52, "BeeWatch", this->Bigfont);
	this->updateNetWork(63, 0, false);
	zigbee.appReg(EndApp);
	zigbee.startReq();
	Z_stack::START_COMFIRM_STATUS startStatus;
	
	while(1){
		if(zigbee.getCallBack() == Z_stack::Zb_startConfirm){
			startStatus = zigbee.startComfirm();
			break;
		}
	}
	lcd.setScreen(BLACK);
}

void BeeWatch::sendPPM(void){
	if(this->isNewPPM == true){
	__IO Z_stack::STATUS zbStt;	
	this->isNewPPM = false;
	Z_stack::TxPacket_s txPacket;
	txPacket.cmdID = HEART_RATE;
	txPacket.dstAddr = 0x0000;
	txPacket.handle = 2;
	txPacket.len = 1;
	txPacket.txPtr = &this->heartRate;
	
	zigbee.sendDataReq(txPacket, false, 10);
		while(1){
			if(zigbee.getCallBack() == Z_stack::Zb_sendDataConfirm){
				zbStt = zigbee.sendDataComfirm(txPacket);
				break;
			}
		}		
	}
}

void BeeWatch::sendMessage(void){
	uint8_t data[9] = {0};
	__IO Z_stack::STATUS zbStt;	

	data[0] = 0xFF;
	for(uint8_t i = 0; i < 4; i++){
		data[i + 1] = actMin >> 8 * i;
		data[i + 5] = inActMin >> 8 * i;		
	}
	
	Z_stack::TxPacket_s txPacket;
	txPacket.cmdID = STATUS;
	txPacket.dstAddr = 0x0000;
	txPacket.handle = 1;
	txPacket.len = 9;
	txPacket.txPtr = data;
	
		zigbee.sendDataReq(txPacket, false, 10);
		while(1){
			if(zigbee.getCallBack() == Z_stack::Zb_sendDataConfirm){
				zbStt = zigbee.sendDataComfirm(txPacket);
				break;
			}
		}
}

void BeeWatch::sendAlert(void){
	__IO Z_stack::STATUS zbStt;	
	
	Z_stack::TxPacket_s txPacket;
	txPacket.cmdID = FALL_ALERT;
	txPacket.dstAddr = 0x0000;
	txPacket.handle = 2;
	txPacket.len = 0;
	
		zigbee.sendDataReq(txPacket, false, 10);
		while(1){
			if(zigbee.getCallBack() == Z_stack::Zb_sendDataConfirm){
				zbStt = zigbee.sendDataComfirm(txPacket);
				break;
			}
		}
}

void BeeWatch::checkGyroStatus(void){
	if(PB1.read() == 1){
		gyro.readInterrupt(this->rawStatus);
	}
}

void BeeWatch::drawBattery(uint8_t x, uint8_t y, BATTERY_LEVEL batLevel){
	this->beePicture.width = 32;
	this->beePicture.height = 16;

	switch(batLevel){
		case LOW:
			this->beePicture.code = Low_Battery32x16;
			lcd.drawPicture(x, y, this->beePicture);
			break;
		case MEDIUM:
			this->batteryBitmap.code = Medium_Battery32x16;
			lcd.drawBitmap(x, y, this->batteryBitmap);
			break;
		case FULL:
			this->batteryBitmap.code = Full_Battery32x16;
			lcd.drawBitmap(x, y, this->batteryBitmap);
			break;
		case CHARGING:
			this->beePicture.code = Charging_Battery32x16;
			lcd.drawPicture(x, y, this->beePicture);
			break;
	}
}

void BeeWatch::updateHeartRate(uint8_t x, uint8_t y){
	char buff[5];
	if(ppm.getHeartRate(this->heartRate) == true){
		this->isNewPPM = true;
		sprintf((char*)buff, " %0.2d", this->heartRate);
		lcd.putStr(x, y, buff, this->smallFont);
	}
}

void BeeWatch::updateTime(uint8_t x, uint8_t y){
	uint8_t timeData[10];
	_RTC::time_s nowTime;
	
	clock.getTime(nowTime);
		
	if(this->time.hours != nowTime.hours){
		sprintf((char*)timeData, "%0.2d:", nowTime.hours);
		lcd.putStr(x, y, (const char*)timeData, this->smallFont);
	}
	if(this->time.minutes != nowTime.minutes){
		sprintf((char*)timeData, "%0.2d:", nowTime.minutes);
		lcd.putStr(x + 7*2 + 4, y, (const char*)timeData, this->smallFont);
	}
	if(this->time.seconds != nowTime.seconds){
		sprintf((char*)timeData, "%0.2d", nowTime.seconds);
		lcd.putStr(x + 7*2 + 7*2 + 8, y, (const char*)timeData, this->smallFont);
	}
	this->time = nowTime;
}

void BeeWatch::updateStatus(uint8_t x, uint8_t y){
		if(this->rawStatus.isFreeFall == true){
			if(gyro.humanFallDetect() == ADXL345::NORMAL_FALL){
				this->status.isFreeFall = true;
			} else {
				this->status.isActivity = true;
			};
		} else {
			this->status = this->rawStatus;
		}	
	
	if(this->status.isActivity == true && this->oldStatus != ACTIVITY && freeFallTime == 0){
		this->smallFont.textColor = ORANGE;
		lcd.putStr(x, y, "ACTIVITY  ", this->smallFont);
		this->smallFont.textColor = WHITE;
		this->oldStatus = ACTIVITY;
		actFlag = true;
	}
	if(this->status.isInactivity == true && this->oldStatus != INACTIVITY && freeFallTime == 0){
		lcd.putStr(x, y, "INACTIVITY", this->smallFont);
		this->oldStatus = INACTIVITY;
		actFlag = false;
	}
	if(this->status.isFreeFall == true && this->oldStatus != FREE_FALL){
		this->sendAlert();
		this->smallFont.textColor = RED;
		lcd.putStr(x, y, "FREE FALL", this->smallFont);
		this->smallFont.textColor = WHITE;
		this->oldStatus = FREE_FALL;
		freeFallTime = 5000;
		Sys_subISRAssign(FreeFallCount);
	}	
}

void BeeWatch::ActivitySttScreen(uint32_t actTime, uint32_t inActTime){
	__IO uint8_t actPercent, inActPercent;
	char buff[20] = {0};
	char buff1[20] = {0};
	
	actPercent = (actTime * 100 / (inActTime + actTime));
	actPercent = 128 * actPercent / 100;
	inActPercent = 128 - actPercent;
	
	this->updateStatus(20, 0);
	
	if(actPercent == 0){
		lcd.setAddress(0, 32, 127, 32 + 32 - 1);		
		for(uint16_t i = 0; i < 32*128; i++){
			lcd.sendWord(WHITE);
		}
	}

	if(inActPercent == 0){
		lcd.setAddress(0, 32, 127, 32 + 32);
		for(uint16_t i = 0; i < 32*128; i++){
			lcd.sendWord(ORANGE);
		}
	}
	
	if(inActPercent != 0 && actPercent != 0){
		lcd.setAddress(0, 32, actPercent - 1, 32 + 32);
		for(uint16_t i = 0; i < actPercent*32; i++){
			lcd.sendWord(ORANGE);			
		}
		lcd.setAddress(actPercent, 32, actPercent + inActPercent - 1, 32 + 32);
		for(uint16_t i = 0; i < inActPercent*32; i++){
			lcd.sendWord(WHITE);			
		}
	}
	sprintf((char*)buff, "Activity %0.2dh %0.2dm", actTime / 60, actTime % 60);
	lcd.putStr(0, 82, buff, this->smallFont);
	sprintf((char*)buff, "InActivity %0.2dh %0.2dm", inActTime / 60, inActTime % 60);
	lcd.putStr(0, 100, buff, this->smallFont);

}

void BeeWatch::drawHeart(uint8_t x, uint8_t y){
	this->beePicture.width = 32;
	this->beePicture.height = 32;
	this->beePicture.code = heartcolor32x32;
	lcd.drawPicture(x, y, this->beePicture);
}

uint32_t BeeWatch::getActMin(void){
	return actMin;
}
uint32_t BeeWatch::getInActMin(void){
	return inActMin;
}


void BeeWatch::updateNetWork(uint8_t x, uint8_t y, bool isConnected){
	this->beePicture.width = 32;
	this->beePicture.height = 16;
	
	if(isConnected == true){
		this->beePicture.code = NetworkConnect32x16;
		lcd.drawPicture(x, y, this->beePicture);
	} else {
		this->beePicture.code = NetworkFailed32x16;
		lcd.drawPicture(x, y, this->beePicture);
	}
}

ADXL345* BeeWatch::getGyroInstant(void){ 
	return &gyro;
}
ILI9163* BeeWatch::getLCDInstant(void){
	return &lcd;
}

}