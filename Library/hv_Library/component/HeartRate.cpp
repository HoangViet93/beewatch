/**
  ******************************************************************************
 * @file    HeartRate.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    2-12-2015
 * @brief   Heart Rate mesuare driver 
  */
//-------------------------------------------------------------------------
#include "HeartRate.h"
#include "string.h"

namespace hv_driver {
/**
	* @brief  Heart Rate class constructor allocated data member
	* @param  ADC_TypeDef* ADCx - ADC peripheral instance
	* @param  uint32_t ADC_CHANNEL_x - ADC channel
	* @param  GPIO_TypeDef* sensorPort, uint16_t sensorPin
	* @retval none
	*/
HeartRate::HeartRate(ADC_TypeDef *ADCx, uint32_t ADC_CHANNEL_x,
										 GPIO_TypeDef* sensorPort, uint16_t sensorPin) {
	_ADC_CHANNEL_x = ADC_CHANNEL_x;
	_sensorPort = sensorPort;
	_sensorPin = sensorPin;

	_processDone = false;
	_pulseValueCount = 0;
	_step = 0;
	_startTick = 0;
	_endTick = 0;
	_init = false;
}

HeartRate::~HeartRate(void) {
}

/**
	* @brief  init ADC periph, sensor pin
	* @param  none
	* @retval none
	*/
void HeartRate::init(void) {
	/*	Reconfig ADC peripherals	*/
	_AdcHandle_p.Instance = ADC1;
	_AdcHandle_p.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	_AdcHandle_p.Init.ScanConvMode = ADC_SCAN_DISABLE;
	_AdcHandle_p.Init.ContinuousConvMode = ENABLE;
	_AdcHandle_p.Init.DiscontinuousConvMode = DISABLE;
	_AdcHandle_p.Init.NbrOfConversion = 1;
	_AdcHandle_p.Init.NbrOfDiscConversion = 1;
	_AdcHandle_p.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONVEDGE_NONE;

	/* Enable ADC Clock	*/
  RCC_PeriphCLKInitTypeDef  PeriphClkInit;
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
  __HAL_RCC_ADC1_CLK_ENABLE();

  /*	Enable Sensor Pin port Clock	*/
  if (_sensorPort == GPIOA) {
		__GPIOA_CLK_ENABLE();
	} else if (_sensorPort == GPIOB) {
		__GPIOB_CLK_ENABLE();
	} else if(_sensorPort == GPIOC) {
		__GPIOC_CLK_ENABLE();
	}
  /*	config sensorPin peripheral	*/
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = _sensorPin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(_sensorPort, &GPIO_InitStruct);

  /*	Init ADC */
	HAL_ADC_Init(&_AdcHandle_p);
	
	_sConf.Channel = _ADC_CHANNEL_x;
	_sConf.Rank = ADC_REGULAR_RANK_1;
	_sConf.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;

	_init = true;
}

/**
	* @brief  reading ADC value of sensorPin
	* @param  none
	* @retval sensor ADC value
	* @note		call init methods first
	*/
uint16_t HeartRate::readADC(void) {
	ADC_ChannelConfTypeDef sConf;

	/*	config read channel */
	sConf.Channel = _ADC_CHANNEL_x;
	sConf.Rank = ADC_REGULAR_RANK_1;
	sConf.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	HAL_ADC_ConfigChannel(&_AdcHandle_p, &sConf);

	/* start conversasion	*/
	 HAL_ADC_Start(&_AdcHandle_p);
	
	 while(HAL_IS_BIT_CLR(_AdcHandle_p.Instance->SR, ADC_FLAG_EOC)) {
		 return HAL_ADC_GetValue(&_AdcHandle_p);
	}
	
//	/* wait for conversasion done with timeOut = 10 */
//	if (HAL_ADC_PollForConversion(_AdcHandle_p, 10) == HAL_OK) {
//		/* Get the converted value of regular channel */
//		return HAL_ADC_GetValue(_AdcHandle_p);
//	}

	return 0;
}

/**
	* @brief  get heart pulse per min value
	* @param  uint8_t &ppmValue - output ppm value
	* @retval true if processing done and new output data
	* @note		processing methods must be call with sampling time (should be 1ms)
	*/
bool HeartRate::getHeartRate(uint8_t &ppmValue) {
	uint16_t sum = 0;
	float pulseValue = 0;	
	
	if (_processDone != true ) {	// processing pulse not done
		return false;
	}

	/*	Get Average Pulse value	*/
	for(uint8_t i = 0; i < HEART_RATE_PULSE_SAMPLE; i++) {
		sum += _tmpPulseValue[i];
	}
	pulseValue = sum / HEART_RATE_PULSE_SAMPLE;

	ppmValue = 60000 / pulseValue; // get pulse per min

	_processDone = false; // reset flag
	return true;
}

/**
	* @brief  sampling ADC value and compare with high pulse thresshold
	* @param  none
	* @retval true if high pulse detected
	*/
bool HeartRate::getHighPulse(void) {
	/* Sampling ADC value */
	for(uint8_t i = (HEART_RATE_ADC_SAMPLE - 1); i > 0; i--) {
		_tmpADCValue[i] = _tmpADCValue[i - 1];
	}
	_tmpADCValue[0] = readADC();

	/*	check all samples reach the high pulse thresshold */
	for (uint8_t i = 0; i < HEART_RATE_ADC_SAMPLE; i++) {
		if(_tmpADCValue[i] < HEART_RATE_HIGH_PULSE_END || _tmpADCValue[i] > HEART_RATE_HIGH_PULSE_TOP) {
			return false;
		}
	}

	memset(_tmpADCValue, 0, HEART_RATE_ADC_SAMPLE);	// reset buffer
	return true;
}

/**
	* @brief  sampling ADC value and compare with low pulse thresshold
	* @param  none
	* @retval true if low pulse detected
	*/
bool HeartRate::getLowPulse(void) {
	/* Sampling ADC value */
	for(uint8_t i = (HEART_RATE_ADC_SAMPLE - 1); i > 0; i--) {
		_tmpADCValue[i] = _tmpADCValue[i - 1];
	}
	_tmpADCValue[0] = readADC();

	/*	check all samples reach the low pulse thresshold */
	for (uint8_t i = 0; i < HEART_RATE_ADC_SAMPLE; i++) {
		if(_tmpADCValue[i] > HEART_RATE_LOW_PULSE_TOP ) {
			return false;
		}
	}

	memset(_tmpADCValue, 0, HEART_RATE_ADC_SAMPLE); // reset buffer
	return true;
}

/**
	* @brief  processing samples value and get the pulse time
	* @param  none
	* @retval none
	* @note 	this methods must be call with sampling time(1ms)
	* 				to enable measuare process. Data is ready if at least
	* 				HEART_RATE_PULSE_SAMPLE is ready in _tmpPulseValue.
	*/
void HeartRate::processing(void) {
	if(_init != true) {
		return;
	}
	/* step 1 detect first high pulse */
	if (_step == 0) {
		if (getHighPulse() == true) {
			_step = 1; // jump to step 2
			_startTick = Sys_getTick(); // get first pulse time
		}
	}

	/* step 2 detect low pulse	*/
	if (_step == 1) {
		if(getLowPulse() == true) {
			_step = 2;	// jump to step 3
		} else if (Sys_getTick() - _startTick > HEART_RATE_HIGH_PULSE_MAX_TIME) {
			_step = 0; // if time overflow goback step 1
		}
	}

	/* step 3 detect second high pulse calculate pulse value	*/
	if (_step == 2) {
		if (getHighPulse() == true) {
			_step = 0;
			_endTick = Sys_getTick() - _startTick; // get pulse value
			/* check condition of pulse range */
			if (_endTick > HEART_RATE_PULSE_MIN_TIME && _endTick < HEART_RATE_PULSE_MAX_TIME){
				_tmpPulseValue[_pulseValueCount] = (_endTick - 2 * HEART_RATE_ADC_SAMPLE); // get pulse sample
				_pulseValueCount++; // point to next Pulse Value Sample
				if (_pulseValueCount == HEART_RATE_PULSE_SAMPLE) { // sample Pulse done
					_processDone = true;
					_pulseValueCount = 0;
				}
			}
		} else if (Sys_getTick() - _startTick > HEART_RATE_PULSE_MAX_TIME) {
			_step = 0; // if time overflow goback step 1
		}
	}
}

}
