/**
  ******************************************************************************
 * @file    RTC.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    12-6-2015
 * @brief   RTC driver
  */
//-------------------------------------------------------------------------
#include "RTC.h"

#define RTC_STATUS_TIME_OK	0x1234
#define RTC_LEAP_YEAR(year)             ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))

/* Days in a month */
static uint8_t RTC_Months[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Not leap year */
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	/* Leap year */
};

namespace hv_driver {

_RTC::_RTC(void){
}		
	
bool _RTC::init(void){
	uint16_t BKPstatus;

	/*	Config RTC time Async	*/
	rtcHandle.Instance = RTC;
	rtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;

	/* Enable PWR peripheral clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Allow access to BKP Domain */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_BKP_CLK_ENABLE();

  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*	Configue LSE as RTC clock soucre */
	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

//  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
//  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
//  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);


  /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();

  BKPstatus = HAL_RTCEx_BKUPRead(&rtcHandle, RTC_BKP_DR1);
  if (BKPstatus == RTC_STATUS_TIME_OK) { // DR register already write no need to config date and tim

  	/* Wait for RTC APB registers synchronisation (needed after start-up from Reset) */
  	HAL_RTC_WaitForSynchro(&rtcHandle);

  	/* Clear reset flags */
  	__HAL_RCC_CLEAR_RESET_FLAGS();

  	/*	Init RTC peripherals	*/
  	HAL_RTC_Init(&rtcHandle);

  	return true;
  } else { // DR register not write before set default date and time
  	date_s date_s;
  	time_s time_s;

  	date_s.day = 22;
  	date_s.month = 3;
  	date_s.weekday = SATURDAY;
  	date_s.year = 2016;
  	setDate(date_s);

  	time_s.hours = 3;
  	time_s.minutes = 21;
  	time_s.seconds = 0;
  	setTime(time_s);

  	/*	Init RTC peripherals	*/
  	HAL_RTC_Init(&rtcHandle);
  	/*	write Backup register */
  	HAL_RTCEx_BKUPWrite(&rtcHandle, RTC_BKP_DR1, RTC_STATUS_TIME_OK);

  	return false;	
	}
}	
	

bool _RTC::setDate(date_s &date_s) {
	RTC_DateTypeDef rtcDate;
	uint8_t year = date_s.year - 2000;

	/* check a valid date data */
	if(date_s.month > 12 ||
		 year > 99 ||
		 date_s.weekday > 6 ||
		 date_s.day == 0 ||
		 date_s.day > RTC_Months[RTC_LEAP_YEAR(date_s.year) ? 1 : 0][date_s.month - 1]) {
			return false;
	}

	/* Fill date	*/
	rtcDate.Date = date_s.day;
	rtcDate.Month = date_s.month;
	rtcDate.WeekDay = (WEEKDAY)date_s.weekday;
	rtcDate.Year = year;

	HAL_RTC_SetDate(&this->rtcHandle, &rtcDate, RTC_FORMAT_BIN);

	return true;
}

bool _RTC::setDate(WEEKDAY weekday, uint8_t day, uint8_t month, uint16_t year) {
	date_s date_s;

	date_s.weekday = weekday;
	date_s.day = day;
	date_s.month = month;
	date_s.year = year;

	return setDate(date_s);
}

bool _RTC::setTime(time_s &time_s) {
  RTC_TimeTypeDef rtcTime;
  if (time_s.seconds > 59 ||
  		time_s.minutes > 59 ||
			time_s.hours > 23) {
  	return false;
  }
  rtcTime.Seconds = time_s.seconds;
  rtcTime.Minutes = time_s.minutes;
  rtcTime.Hours = time_s.hours;

  HAL_RTC_SetTime(&this->rtcHandle, &rtcTime, RTC_FORMAT_BIN);

  return true;
}

bool _RTC::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	time_s time_s;
	time_s.seconds = seconds;
	time_s.minutes = minutes;
	time_s.hours = hours;

	return setTime(time_s);
}

/**
	* @brief  get date data
	* @param  rtc_date_s& date_s - output date struct
	* @retval true if get date success
	*/
bool _RTC::getDate(date_s& date_s) {
	RTC_DateTypeDef rtcDate;
	if (HAL_RTC_GetDate(&this->rtcHandle, &rtcDate, RTC_FORMAT_BIN) == HAL_OK) {
		date_s.day = rtcDate.Date;
		date_s.month = rtcDate.Month;
		date_s.weekday = (WEEKDAY)rtcDate.WeekDay;
		date_s.year = rtcDate.Year + 2000;

		return true;
	} else {
		return false;
	}
}

/**
	* @brief  get time data
	* @param  rtc_date_s& time_s - output time struct
	* @retval true if get time success
	*/
bool _RTC::getTime(time_s &time_s) {
	RTC_TimeTypeDef rtcTime;
	if (HAL_RTC_GetTime(&this->rtcHandle, &rtcTime, RTC_FORMAT_BIN) == HAL_OK) {
		time_s.seconds = rtcTime.Seconds;
		time_s.minutes = rtcTime.Minutes;
		time_s.hours = rtcTime.Hours;

		return true;
	} else {
		return false;
	}
}

} /* hv_driver */