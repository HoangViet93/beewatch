/**
  ******************************************************************************
 * @file    main.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    2-12-2015
 * @brief   BeeWatch main program
  */
//-------------------------------------------------------------------------
#include "stm32f1xx.h"
#include "GPIO.h"
#include "SPI.h"
#include "ILI9163.h"
#include "RTC.h"
#include "BeeWatch.h" 
#include "MISC.h"
#include "cmsis_os.h"

using namespace hv_driver;

/* Local Function prototype */
void checkStatus(void);

static void MainScreen(void const *argument);
static void Network(void const *argument);
static void ActivityStatus(void const *argument);
static void updateTime(void const *argument);

/* Local Object */
GPIO PC13(GPIOC, GPIO::PIN13); // led pin
osThreadId ThreadMainScreen, ThreadNetwork, ThreadActivity, ThreadTime;

BeeWatch _BeeWatch;

/* Local Var */
uint16_t count = 0;

int main(void){
	Sys_init();
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	PC13.initOutput(GPIO::PP, GPIO::MEDIUM); // init LED
	PC13.set();
	_BeeWatch.init();
	_BeeWatch.initGyro();
	_BeeWatch.initZigbee();
	PC13.reset();

  osThreadDef(MAIN_SCREEN, MainScreen, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(ACTIVITY_STATUS, ActivityStatus, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(NETWORK, Network, osPriorityHigh, 0, configMINIMAL_STACK_SIZE);
 
  ThreadMainScreen = osThreadCreate(osThread(MAIN_SCREEN), NULL);
	ThreadActivity = osThreadCreate(osThread(ACTIVITY_STATUS), NULL);
	ThreadNetwork = osThreadCreate(osThread(NETWORK), NULL);
	
  osKernelStart();
	while(1){
	}
}

static void MainScreen(void const *argument){
	(void) argument;
	__IO uint8_t buff[10];
	_BeeWatch.drawBattery(95, 0, BeeWatch::CHARGING);
	_BeeWatch.updateNetWork(63, 0, true);
	_BeeWatch.drawHeart(0, 20);
	while(1){
		sprintf((char*)buff, "act min %0.3d", _BeeWatch.getInActMin());
		_BeeWatch.getLCDInstant()->putStr(0, 93, (const char*)buff, _BeeWatch.getsmallFont());
		
		sprintf((char*)buff, "inact min %0.3d", _BeeWatch.getActMin());
		_BeeWatch.getLCDInstant()->putStr(0, 110, (const char*)buff, _BeeWatch.getsmallFont());
		_BeeWatch.updateTime(0, 60);
		osDelay(1000);
	}
}

static void Network(void const *argument){
	(void) argument;
	while(1){
		_BeeWatch.sendMessage();
		_BeeWatch.sendPPM();
		osDelay(3000);
	}
}

static void ActivityStatus(void const *argument){
	(void) argument;
	Sys_subISRAssign(checkStatus);
	while(1){
		_BeeWatch.updateStatus(0, 76);
		_BeeWatch.updateHeartRate(35, 28);
		osDelay(10);
	}
}

void checkStatus(void){
	_BeeWatch.checkGyroStatus();
}

