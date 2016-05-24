/**
  ******************************************************************************
 * @file    HeartRate.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    2-12-2015
 * @brief   Heart Rate mesuare driver 
  */
//-------------------------------------------------------------------------

#ifndef HEART_RATE_H
#define HEART_RATE_H

#include "stm32f1xx.h"
#include "MISC.h"
	
namespace hv_driver {	
class HeartRate {
public:	
	enum HEART_PARAM {	
		HEART_RATE_ADC_SAMPLE 	= 10,	
		HEART_RATE_PULSE_SAMPLE = 10, 

		HEART_RATE_HIGH_PULSE_MAX_TIME = 400, // in ms
		HEART_RATE_PULSE_MAX_TIME = 1700,
		HEART_RATE_PULSE_MIN_TIME = 500,

		HEART_RATE_LOW_PULSE_TOP	= 750,

		HEART_RATE_HIGH_PULSE_TOP	= 2550,
		HEART_RATE_HIGH_PULSE_END	= 2000,
	};
public:
	HeartRate(ADC_TypeDef *ADCx, uint32_t ADC_CHANNEL_x, GPIO_TypeDef* sensorPort, uint16_t sensorPin);
	~HeartRate(void);

	void init(void);
	void processing(void);
	bool getHeartRate(uint8_t &ppmValue);
	uint16_t readADC(void);
private:
	/*	private function	*/
	bool getHighPulse(void);
	bool getLowPulse(void);

	/* private variable */
	ADC_HandleTypeDef _AdcHandle_p;
	uint32_t _ADC_CHANNEL_x;
	GPIO_TypeDef* _sensorPort;
	uint16_t _sensorPin;
	ADC_ChannelConfTypeDef _sConf;
	

	bool _processDone; // processing done flag
	bool _init;

	uint16_t _tmpADCValue[HEART_RATE_ADC_SAMPLE];
	uint16_t _tmpPulseValue[HEART_RATE_PULSE_SAMPLE];
	uint8_t  _pulseValueCount;
	uint8_t _step;
	uint32_t _startTick, _endTick;	
};

} /* hv_driver */
#endif /* HEART_RATE_H */