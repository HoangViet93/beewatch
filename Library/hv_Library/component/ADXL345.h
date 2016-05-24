/**
  ******************************************************************************
 * @file    ADXL345.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    11-11-2015
 * @brief   ADXL345 accelerometer driver
  */
//------------------------------------------------------------------------

#ifndef H
#define H

#include "stm32f1xx.h"
#include "GPIO.h"
#include "MISC.h"

namespace hv_driver {

class ADXL345 {
public:	
/* Registers */
enum REGISTER{
	DEVID_REG =           0x00,
	THRESH_TAP_REG =      0x1D,
	OFSX_REG =            0x1E,
	OFSY_REG =            0x1F,
	OFSZ_REG =            0x20,
	DUR_REG =             0x21,
	LATENT_REG =          0x22,
	WINDOW_REG =          0x23,
	THRESH_ACT_REG =      0x24,
 	THRESH_INACT_REG =    0x25,
	TIME_INACT_REG =      0x26,
	ACT_INACT_CTL_REG =   0x27,
	THRESH_FF_REG =       0x28,
	TIME_FF_REG =         0x29,
	TAP_AXES_REG =        0x2A,
	ACT_TAP_STATUS_REG =  0x2B,
	BW_RATE_REG =         0x2C,
	POWER_CTL_REG =       0x2D,
	INT_ENABLE_REG =      0x2E,
	INT_MAP_REG =         0x2F,
	INT_SOURCE_REG =      0x30,
	DATA_FORMAT =         0x31,
	DATAX0_REG =          0x32,
	DATAX1_REG =          0x33,
	DATAY0_REG =          0x34,
	DATAY1_REG =          0x35,
	DATAZ0_REG =          0x36,
	DATAZ1_REG =          0x37,
	FIFO_CTL =            0x38,
	FIFO_STATUS =         0x39
};

/* activity interrupt */
enum INT_TYPE{
	DATA_READY         = 0x07,
	SINGLE_TAP         = 0x06,
	DOUBLE_TAP         = 0x05,
	ACTIVITY           = 0x04,
	INACTIVITY         = 0x03,
  FREE_FALL          = 0x02,
  WATERMARK          = 0x01,
  OVERRUN            = 0x00
};

/* data range */
enum RANGE{
  RANGE_16G          = 0x03,
  RANGE_8G           = 0x02,
  RANGE_4G           = 0x01,
  RANGE_2G           = 0x00
};

/* Data rate codes */
enum DATA_RATE{
	RATE_3200HZ =      0x0F,
	RATE_1600HZ =      0x0E,
	RATE_800HZ =       0x0D,
	RATE_400HZ =       0x0C,
	RATE_200HZ =       0x0B,
	RATE_100HZ =       0x0A,
	RATE_50HZ =        0x09,
	RATE_25HZ =        0x08,
	RATE_12HZ5 =       0x07,
	RATE_6HZ25=        0x06
};

enum I2C_CMD{
	ADXL_WRITE = 0xA6,
	ADXL_READ =  0xA7,
};

/* adxl345 axis type */
enum AXIS{
	AXIS_X = 0x00,
	AXIS_Y = 0x01,
	AXIS_Z =  0x02
};

/* Interrupt pin */
enum INT_PIN{
	INT_PIN_1,
	INT_PIN_2
};

enum HUMAN_FALL {
	NO_FALL = 0,
	NORMAL_FALL, 
	CRITICAL_FALL
};

/* Interrupt read status */
typedef struct {
  bool isOverrun;
  bool isWatermark;
  bool isFreeFall;
  bool isInactivity;
  bool isActivity;
  bool isDoubleTap;
  bool isTap;
  bool isDataReady;
} IntVal_s;

/*	Axis read value	*/
typedef struct {
	float X;
	float Y;
	float Z;
} AxisValue_s;

/*	Tap read status	*/
typedef struct {
  bool isTapOnX;
  bool isTapOnY;
  bool isTapOnZ;
  bool isActivityOnX;
  bool isActivityOnY;
	bool isActivityOnZ;
} TapStatus_s;

public:
	ADXL345(GPIO* intPin);

	bool init(RANGE range, DATA_RATE dataRate);
	HUMAN_FALL humanFallDetect(void);
	/* common */
	uint8_t  getDevID(void) ;
	uint16_t getOffSet(AXIS axis) ;
			void setOffSet(AXIS axis, uint16_t value_mg) ;
			void setDataRate(DATA_RATE dataRate) ;
			void useInterrupt(INT_PIN intPin) ;
			void readInterrupt(IntVal_s& intVal) ;
			void setRange(RANGE range) ;
			void readAxisValue(AxisValue_s& AxisVal) ;
			void clearSettings(void) ;

	/* tap and double tap control */
	uint16_t getTapThressHold(void) ;
			void setTapThressHold(uint16_t value_mg) ;
	 uint8_t getTapDuration(void) ;
			void setTapDuration(uint8_t duration_ms) ;
	uint16_t getLatencyTime(void) ;
			void setLatencyTime(uint16_t time_ms) ;
	uint16_t getWindowTime(void) ;
			void setWindowTime(uint16_t time_ms) ;
			bool getTapAxes(AXIS axis) ;
			void setTapAxes(AXIS axis, bool state) ;
			void setDoubleTap(bool state) ;
			void getActTapStatus(TapStatus_s& tapStatus) ;

	/* activity control */
	uint16_t getThreshActivity(void) ;
			void setThreshActivity(uint16_t actThresh_mg) ;
			void setActControl(AXIS axis, bool state) ;
			bool getActControl(AXIS axis) ;

	/* inactivity control */
	uint16_t getThreshInactivity(void) ;
			void setThreshInactivity(uint16_t inactThresh_mg) ;
	 uint8_t getInactivityTime(void) ;
			void setInactivityTime(uint8_t time_s) ;
			void setInactControl(AXIS axis, bool state) ;
			bool getInactControl(AXIS axis) ;

	/* free-fall control */
	uint16_t getThreshFreeFall(void) ;
			void setThreshFreeFall(uint16_t freefallThresh_mg) ;
	uint16_t getFreeFallTime(void) ;
			void setFreeFallTime(uint16_t time_ms) ;

	/* Communicate */
		  void writeByte(uint8_t address, uint8_t wData);
	 uint8_t readByte(uint8_t address);
		  void writeMultiByte(uint8_t startAddress, uint8_t* txPtr, uint8_t size);
		  void readMultiByte(uint8_t startAddress, uint8_t* rxPtr, uint8_t size);
			void writeBit(uint8_t address, uint8_t bitPos, bool state);
			bool readBit(uint8_t address, uint8_t bitPos);
private:
  GPIO* intPin;
  I2C_HandleTypeDef i2c2;
};

} /* hv_driver namespace */

#endif /* ADXL345_H */