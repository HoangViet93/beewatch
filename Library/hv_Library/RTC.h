/**
  ******************************************************************************
 * @file    RTC.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    12-6-2015
 * @brief   RTC driver
  */
//-------------------------------------------------------------------------

#ifndef _RTC_H
#define _RTC_H

#include "stm32f1xx.h"

namespace hv_driver {

class _RTC {
public:
enum WEEKDAY {
	MONDAY = 0x01,
	TUESDAY = 0x02,
	WEDNESDAY = 0x03,
	THURSDAY = 0x04,
	FRIDAY = 0x05,
	SATURDAY = 0x06,
	SUNDAY = 0x00
};

typedef struct {
	WEEKDAY weekday;
	uint8_t day;
	uint8_t month;
	uint16_t year;
} date_s;

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
} time_s;
public:
	_RTC(void);

	bool init(void);

	bool setDate(date_s &date);
	bool setDate(WEEKDAY weekday, uint8_t day, uint8_t month, uint16_t year);
	bool setTime(time_s &time_s);
	bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

	bool getDate(date_s &date_s);
	bool getTime(time_s &time_s);

private:	
	RTC_HandleTypeDef rtcHandle;
};
} /* hv_driver namespace */

#endif /* _RTC_H */