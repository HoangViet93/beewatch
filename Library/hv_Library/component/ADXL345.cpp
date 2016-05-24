/**
  ******************************************************************************
 * @file    ADXL345.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    11-11-2015
 * @brief   ADXL345 accelerometer driver
  */
//-------------------------------------------------------------------------
#include "ADXL345.h"

namespace hv_driver {

ADXL345::ADXL345(GPIO* intPin){
	this->intPin = intPin;
}

bool ADXL345::init(RANGE range, DATA_RATE dataRate){
	GPIO PB10(GPIOB, GPIO::PIN10);
	GPIO PB11(GPIOB, GPIO::PIN11);
	
	PB10.init(GPIO::AF, GPIO::OD, GPIO::NONE, GPIO::MEDIUM);
	PB11.init(GPIO::AF, GPIO::OD, GPIO::NONE, GPIO::MEDIUM);
	
	/* Init I2C peripherals */
	__HAL_RCC_I2C2_CLK_ENABLE();	

	i2c2.Instance             = I2C2;
	i2c2.Init.ClockSpeed      = 100000; // clock speed 100kHz
	i2c2.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	i2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
	i2c2.Init.DutyCycle	      = I2C_DUTYCYCLE_2;
	i2c2.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
	i2c2.Init.OwnAddress1     = 0x00;
	i2c2.Init.OwnAddress2     = 0x00;
	i2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	HAL_I2C_Init(&i2c2);
	if(this->getDevID() != 0xE5){
		return false;
	}
	this->writeByte(POWER_CTL_REG, 0x08); // enable mesuare mode
	this->clearSettings();
	this->setRange(range);
	this->setDataRate(dataRate);
	
	return true;
}

ADXL345::HUMAN_FALL ADXL345::humanFallDetect(void){
	uint32_t actTimeOut, inactTimeOut;
	IntVal_s IntVal;
	HUMAN_FALL fallStatus = NO_FALL;
	
	actTimeOut = Sys_getTick();
	while(Sys_getTick() - actTimeOut < 200){
		if(this->intPin->read() == 1){
			this->readInterrupt(IntVal);
			if(IntVal.isActivity == true){
				inactTimeOut = Sys_getTick();
				while(Sys_getTick() - inactTimeOut < 3500){
					if(this->intPin->read() == 1){
						this->readInterrupt(IntVal);
						if(IntVal.isInactivity == true){
							fallStatus = NORMAL_FALL;
							return fallStatus;
						}	
					}
				}
			}
		}
	}
	return fallStatus;
}
/*	COMMON */
uint8_t ADXL345::getDevID(void){
	return readByte(DEVID_REG);
}

uint16_t ADXL345::getOffSet(AXIS axis){
	uint16_t value;
	switch (axis) {
		case AXIS_X:
			value = readByte(OFSX_REG);
			break;
		case AXIS_Y:
			value = readByte(OFSY_REG);
			break;
		case AXIS_Z:
			value = readByte(OFSZ_REG);
			break;
	}
	return value * 15.6; // add scale factor
}

void ADXL345::setOffSet(AXIS axis, uint16_t value_mg){
	switch (axis) {
		case AXIS_X:
			writeByte(OFSX_REG, (uint8_t)(value_mg / 15.6));
			break;
		case AXIS_Y:
			writeByte(OFSY_REG, (uint8_t)(value_mg / 15.6));
			break;
		case AXIS_Z:
			writeByte(OFSZ_REG, (uint8_t)(value_mg / 15.6));
			break;
	}
}

void ADXL345::setDataRate(DATA_RATE dataRate){
	writeByte(BW_RATE_REG, dataRate);
}

void ADXL345::useInterrupt(INT_PIN intPin){
	if(intPin == INT_PIN_1){
		writeByte(INT_MAP_REG, 0x00);	// mapping interrupt for pin
	} else {
		writeByte(INT_MAP_REG, 0xFF);
	}
	writeByte(INT_ENABLE_REG, 0x1C); // enable interrupt
}

void ADXL345::readInterrupt(IntVal_s& intVal){
	uint8_t data = readByte(INT_SOURCE_REG);
	intVal.isActivity   = ((data >> ACTIVITY) & 1);
	intVal.isDataReady  = ((data >> DATA_READY) & 1);
	intVal.isDoubleTap  = ((data >> DOUBLE_TAP) & 1);
	intVal.isFreeFall   = ((data >> FREE_FALL) & 1);
	intVal.isInactivity = ((data >> INACTIVITY) & 1);
	intVal.isOverrun    = ((data >> OVERRUN) & 1);
	intVal.isTap		    = ((data >> SINGLE_TAP) & 1);
	intVal.isWatermark  = ((data >> WATERMARK) & 1);
}

void ADXL345::setRange(RANGE range) {
	uint8_t data = readByte(DATA_FORMAT);
	data &= 0x00; // reset data format
	data |= range; // write mesuare range
	data |= 0x08; // full-resolution
	writeByte(DATA_FORMAT, data);

}

void ADXL345::readAxisValue(AxisValue_s& AxisVal){
	uint8_t valueBuffer[6];
	readMultiByte(DATAX0_REG, valueBuffer, 6);
	AxisVal.X = valueBuffer[1] << 8 | valueBuffer[0];
	AxisVal.Y = valueBuffer[3] << 8 | valueBuffer[2];
	AxisVal.Z = valueBuffer[5] << 8 | valueBuffer[4];
	// add gravity factor
	//AxisVal.X = AxisVal.X * 0.004 * 9.806;
	//AxisVal.Y = AxisVal.Y * 0.004 * 9.806;
	//AxisVal.Z = AxisVal.Z * 0.004 * 9.806;
}

/**
  * @brief  clear previous setting and set to default value
  * @param  none
  * @return none
  */
void ADXL345::clearSettings(void){
    setRange(RANGE_2G);
    setDataRate(RATE_100HZ);

    writeByte(THRESH_TAP_REG, 0x00);
    writeByte(DUR_REG, 0x00);
    writeByte(LATENT_REG, 0x00);
    writeByte(WINDOW_REG, 0x00);
    writeByte(THRESH_ACT_REG, 0x00);
    writeByte(THRESH_INACT_REG, 0x00);
    writeByte(TIME_INACT_REG, 0x00);
    writeByte(THRESH_FF_REG, 0x00);
    writeByte(TIME_FF_REG, 0x00);
    writeByte(INT_MAP_REG, 0x00);
    writeByte(INT_ENABLE_REG, 0x00);

    uint8_t value;

    value = readByte(ACT_INACT_CTL_REG);
    value &= 0x88;
    writeByte(ACT_INACT_CTL_REG, value);

    value = readByte(TAP_AXES_REG);
    value &= 0xF8;
    writeByte(TAP_AXES_REG, value);
}

/* TAP AND DOUBLE-TAP CONTROL */
/**
  * @brief  get tap thresshold value
  * @param  none
  * @return tap thress value in mg
  */
uint16_t ADXL345::getTapThressHold(void){
	return (uint16_t)(readByte(THRESH_TAP_REG) * 62.5);
}

/**
  * @brief  set tap thresshold
  * @param  uint16_t value_mg - thresshold value in mg
  * @return none
  */
void ADXL345::setTapThressHold(uint16_t value_mg){
	return writeByte(THRESH_TAP_REG, (value_mg / 62.5));
}

/**
  * @brief  get tap duration in milisecond
  * @param  none
  * @return tap duration value
  */
uint8_t ADXL345::getTapDuration(void){
	return (uint8_t)((readByte(DUR_REG) * 625) / 1000);
}

/**
  * @brief  set tap duration in milisecond
  * @param  uint8_t duration_ms - tap duration value in milisecond
  * @return none
  */
void ADXL345::setTapDuration(uint8_t duration_ms){
	writeByte(DUR_REG, (uint8_t)((duration_ms / 625) * 1000));
}

/**
  * @brief  get latency time (the time between detection tap and start
	*					windows time)
  * @param  none
  * @return latency time in milisecond
  */
uint16_t ADXL345::getLatencyTime(void){
	return (uint16_t)((readByte(LATENT_REG) * 1.25));
}

/**
  * @brief  set latency time (the time between detection tap and start
	*					windows time)
  * @param  uint16_t time_ms - latency time in milisecond
  * @return none
  */
void ADXL345::setLatencyTime(uint16_t time_ms){
	writeByte(LATENT_REG, (uint8_t)((time_ms / 1.25)));
}

/**
  * @brief  get windows time (amount of time after the expiration of the 
	* 				latency time during which a second tap valid tap can begin)
  * @param  none 
  * @return windows time value in ms
  */
uint16_t ADXL345::getWindowTime(void){
	return (uint16_t)((readByte(WINDOW_REG) * 1.25));
}

/**
  * @brief  set windows time (amount of time after the expiration of the 
	* 				latency time during which a second tap valid tap can begin)
  * @param  uint16_t time_ms - windows time value in ms
  * @return none
  */
void ADXL345::setWindowTime(uint16_t time_ms){
	writeByte(WINDOW_REG, (uint8_t)((time_ms / 1.25)));
}

/**
  * @brief  get tap axes reg to choose which axis using in tap detection.
  * @param  AXIS axis - axis want to get.
  * @return	true axis already use.
  */
bool ADXL345::getTapAxes(AXIS axis){
	bool state;
	switch (axis) {
		case AXIS_X:
			state = readBit(TAP_AXES_REG, 2);
			break;
		case AXIS_Y:
			state = readBit(TAP_AXES_REG, 1);
			break;
		case AXIS_Z:
			state = readBit(TAP_AXES_REG, 0);
			break;
		default:
			break;
	}
	return state;
}

/**
  * @brief  Set tap axes reg to choose which axis using in tap detection 
  * @param  AXIS axis - axis want to set
  * @param  bool state - true axis will be using, otherwise axis will be ignore
  * @return	none
  */
void ADXL345::setTapAxes(AXIS axis, bool state){
	switch (axis) {
		case AXIS_X:
			writeBit(TAP_AXES_REG, 2, state);
			break;
		case AXIS_Y:
			writeBit(TAP_AXES_REG, 1, state);
			break;
		case AXIS_Z:
			writeBit(TAP_AXES_REG, 0, state);
			break;
		default:
			break;
	}
}

/**
  * @brief  set or reset suppress bit in tap axes register to enable or
	*				  disable second tap dectection in the latency time
  * @param  bool state - true doubetap is enable, false double tap disable
  * @return	none
  */
void ADXL345::setDoubleTap(bool state){
	writeBit(TAP_AXES_REG, 3, ~state);
}

/**
  * @brief  get Activity tap status
  * @param  TapStatus_s& TapStatus - output activity status struct
  * @return none
  */
void ADXL345::getActTapStatus(TapStatus_s& TapStatus){
	uint8_t data = readByte(ACT_TAP_STATUS_REG);

	TapStatus.isActivityOnX = ((data >> 6) & 1);
	TapStatus.isActivityOnY = ((data >> 5) & 1);
	TapStatus.isActivityOnZ = ((data >> 4) & 1);
	TapStatus.isTapOnX = ((data >> 2) & 1);
	TapStatus.isTapOnY = ((data >> 1) & 1);
	TapStatus.isTapOnZ = ((data >> 0) & 1);
}

/* ACTIVITY CONTROL */
/**
  * @brief  get Activity thresshold value 
  * @param  none
  * @return thresshold value in mg
  */
uint16_t ADXL345::getThreshActivity(void){
	return (uint16_t)(readByte(THRESH_ACT_REG) * 62.5);
}

/**
  * @brief  set Activity thresshold value
  * @param  uint16_t actThresh_mg - activity thresshold in mg
  * @return none
  */
void ADXL345::setThreshActivity(uint16_t actThresh_mg){
	writeByte(THRESH_ACT_REG, (uint8_t)(actThresh_mg / 62.5));
}

/**
  * @brief  Set Activity control reg to choose which axis using in activity opreation
  * @param  adxl345_axis_t axis - axis want to set
  * @param  bool state - true axis will be using, otherwise axis will be ignore
  * @return none
  */
void ADXL345::setActControl(AXIS axis, bool state){
	switch (axis) {
		case AXIS_X:
			writeBit(ACT_INACT_CTL_REG, 6, state);
			break;
		case AXIS_Y:
			writeBit(ACT_INACT_CTL_REG, 5, state);
			break;
		case AXIS_Z:
			writeBit(ACT_INACT_CTL_REG, 4, state);
			break;
		default:
			break;
	}
}

/**
  * @brief  get Activity control reg to return which axis currently using in Activity operation
  * @param  adxl345_axis_t axis - axis want to set
  * @return bool state - true axis already using
  */
bool ADXL345::getActControl(AXIS axis){
	bool state;
	switch (axis) {
		case AXIS_X:
			state = readBit(ACT_INACT_CTL_REG, 6);
			break;
		case AXIS_Y:
			state = readBit(ACT_INACT_CTL_REG, 5);
			break;
		case AXIS_Z:
			state = readBit(ACT_INACT_CTL_REG, 4);
			break;
		default:
			break;
	}
	return state;
}

/* INACTIVITY CONTROL */
/**
  * @brief  get inactivity thresshold
  * @param  none
  * @return inactivity thresshold in mg
  */
uint16_t ADXL345::getThreshInactivity(void){
	return (uint16_t)(readByte(THRESH_INACT_REG) * 62.5);
}

/**
  * @brief  set inactivity thresshold
  * @param  uint16_t inactThresh_mg - thresshold value in mg
  * @return none
  */
void ADXL345::setThreshInactivity(uint16_t inactThresh_mg){
	writeByte(THRESH_INACT_REG, (uint8_t)(inactThresh_mg / 62.5));
}

/**
  * @brief  get inactivity time
  * @param  none
  * @return inactivity time in second
  */
uint8_t ADXL345::getInactivityTime(void){
	return readByte(TIME_INACT_REG);
}

/**
  * @brief  set inactivity time
  * @param  uint8_t time_s - inactivity time in second
  * @return none
  */
void ADXL345::setInactivityTime(uint8_t time_s){
	writeByte(TIME_INACT_REG, time_s);
}

/**
  * @brief  set act inact control register to choose which axes 
  *					using in inactivity detection
  * @param  adxl345_axis_t axis - axes using
  * @param  bool state - true axis will be using, false axis will be ignore
  * @return none
  */
void ADXL345::setInactControl(AXIS axis, bool state){
	switch (axis) {
		case AXIS_X:
			writeBit(ACT_INACT_CTL_REG, 2, state);
			break;
		case AXIS_Y:
			writeBit(ACT_INACT_CTL_REG, 1, state);
			break;
		case AXIS_Z:
			writeBit(ACT_INACT_CTL_REG, 0, state);
			break;
		default:
			break;
	}
}

/**
  * @brief  read act inact control register to get which axes 
  *					using in inactivity detection
  * @param  adxl345_axis_t axis - axes want to read
  * @return true axis already using
  */
bool ADXL345::getInactControl(AXIS axis){
	bool state;
	switch (axis) {
		case AXIS_X:
			state = readBit(ACT_INACT_CTL_REG, 2);
			break;
		case AXIS_Y:
			state = readBit(ACT_INACT_CTL_REG, 1);
			break;
		case AXIS_Z:
			state = readBit(ACT_INACT_CTL_REG, 0);
			break;
		default:
			break;
	}
	return state;
}

/* FREE-FALL CONTROL */
/**
  * @brief  Get Free-Fall thresshold
  * @param  none
  * @return Free-Fall thresshold in mg
  */
uint16_t ADXL345::getThreshFreeFall(void){
	return (uint16_t)(readByte(THRESH_FF_REG) * 62.5);
}

/**
  * @brief  Set Free-Fall thresshold
  * @param  uint16_t freefallThresh_mg - free-fall thresshold in mg
  * @return none
  */
void ADXL345::setThreshFreeFall(uint16_t freefallThresh_mg){
	writeByte(THRESH_FF_REG, (uint8_t)(freefallThresh_mg / 62.5));
}

/**
  * @brief  get Free-Fall time duration in milisecond
  * @param  none
  * @return Free-Fall time in milisecond
  */
uint16_t ADXL345::getFreeFallTime(void){
	return (uint16_t)(readByte(TIME_FF_REG) * 5);
}

/**
  * @brief  set Free-Fall time duration in milisecond
  * @param  uint16_t time_ms - Free-Fall time value in milisecond
  * @return none
  */
void ADXL345::setFreeFallTime(uint16_t time_ms){
	writeByte(TIME_FF_REG, (uint8_t)(time_ms / 5));
}

/* COMMUNICATION	*/
/**
  * @brief  Read register Value, set or reset specific bit and write again 
  * @param  uint8_t address - register value
  * @param  uint8_t bitPos - position of specific bit
  * @param  bool state - true bit is set,false bit is reset
  * @return none
  */
void ADXL345::writeBit(uint8_t address, uint8_t bitPos, bool state){
	uint8_t registerValue = readByte(address);
	if (state == true) {
		registerValue |= 1 << bitPos;
	} else {
		registerValue &= ~(1 << bitPos);
	}
	writeByte(address, registerValue);
}

/**
  * @brief  Read register Value, read bit in bitPos
  * @param  uint8_t address - register value
  * @param  uint8_t bitPos - position of specific bit
  * @return value of bit
  */
bool ADXL345::readBit(uint8_t address, uint8_t bitPos){
	uint8_t registerValue = readByte(address);
	return ((registerValue >> bitPos) & 1);
}

void ADXL345::writeByte(uint8_t address, uint8_t wData){
	uint8_t data[2] = {address, wData};
	/* send slave address + write cmd + register address to write to + data */
	HAL_I2C_Master_Transmit(&this->i2c2, (uint16_t)ADXL_WRITE, data, 2, 1000);
}

uint8_t ADXL345::readByte(uint8_t address){
	uint8_t data = 0;
	/* first send Slave address and register address to read form */
	HAL_I2C_Master_Transmit(&this->i2c2, (uint16_t)ADXL_WRITE, &address, 1, 1000);
	/* receive 1 byte data */
	HAL_I2C_Master_Receive(&this->i2c2, (uint16_t)ADXL_READ, &data, 1, 1000);
	return data;
}

void ADXL345::writeMultiByte(uint8_t startAddress, uint8_t* txBuf, uint8_t size){
	uint8_t* pData = new uint8_t [size + 1];
	pData[0] = startAddress;
	for (int index = 1; index <= size; index++) {
		pData[index] = txBuf[index - 1];
	}
	HAL_I2C_Master_Transmit(&this->i2c2, (uint16_t)ADXL_WRITE, pData, size + 1, 1000);
	delete[] pData;
}

void ADXL345::readMultiByte(uint8_t startAddress, uint8_t* rxBuf, uint8_t size){
	/* first send Slave address and register address to read form */
	HAL_I2C_Master_Transmit(&this->i2c2, (uint16_t)ADXL_WRITE, &startAddress, 1, 1000);
	/* receive multi byte data */
	HAL_I2C_Master_Receive(&this->i2c2, (uint16_t)ADXL_READ, rxBuf, size, 1000);
}

} /* hv_driver namespace */
