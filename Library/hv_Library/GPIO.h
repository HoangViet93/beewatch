/**
  ******************************************************************************
 * @file    GPIO.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>, Animal_team
 * @version 1.0
 * @date    20-11-2015
 * @brief   GPIO driver
  */
//-------------------------------------------------------------------------

#ifndef  GPIO_H
#define  GPIO_H

#include "stm32f1xx.h"

namespace hv_driver {

class GPIO {
public:
	enum MODE {
		INPUT = 0x00, OUTPUT = 0x01, AF = 0x02, ANALOG = 0x03, EXT_INT_RISING = 0x04, EXT_INT_FALLING = 0x05, EXT_INT_BOTH = 0x06
	};
	enum PIN {
		PIN0 = 0, PIN1, PIN2, PIN3, PIN4, PIN5, PIN6, PIN7, PIN8, PIN9, PIN10, PIN11, PIN12, PIN13, PIN14, PIN15
	};
	enum OTYPE {
		PP = 0x00, OD = 0x01
	};
	enum SPEED {
		LOW = GPIO_CRL_MODE0_1, MEDIUM = GPIO_CRL_MODE0_0, HIGH = GPIO_CRL_MODE0
	};
	enum PULL {
		NONE = 0x00, UP = 0x01, DOWN = 0x02
	};
public:
	GPIO(GPIO_TypeDef* GPIOx, PIN PINx);
	~GPIO(void);

	void init(MODE mode, OTYPE type, PULL pull, SPEED speed);
	void initOutput(OTYPE type, SPEED speed);
	void initInput(PULL pull);
	void initExti(MODE edge, PULL pull, void (*CallBack)(void));
	void initAF(OTYPE type, SPEED speed);
		
	void set(void);
	void reset(void);
	void toggle(void);
	uint8_t read(void);

	bool enableEXTI(uint8_t priority);
	bool disableEXTI(void);
	bool lock(void);
private:
	MODE mode;
	void (*CallBack)(void);
	GPIO_TypeDef* GPIOx;
	PIN PINx;
};

}

#endif /* GPIO_H	*/

