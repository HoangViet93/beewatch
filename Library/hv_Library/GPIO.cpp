/**
  ******************************************************************************
 * @file    GPIO.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    20-11-2015
 * @brief   GPIO driver
  */
//-------------------------------------------------------------------------
#include "GPIO.h"

bool extiTable[16] = {false}; 
void (*CallBackTable[16])(void) = {NULL};

namespace hv_driver {

GPIO::GPIO(GPIO_TypeDef* GPIOx, PIN PINx){
	this->GPIOx = GPIOx;
	this->PINx = PINx;
}

void GPIO::init(MODE mode, OTYPE type, PULL pull, SPEED speed){
	__IO uint32_t temp;
	__IO uint8_t offset = 0;
	
	this->mode = mode;
	/* clear previous state of pin */
	if(this->PINx < 8){
		this->GPIOx->CRL &= ~((GPIO_CRL_MODE0 + GPIO_CRL_CNF0) << (4 * this->PINx));
		temp = this->GPIOx->CRL;
		offset = 0;
	} else {
		this->GPIOx->CRH &= ~((GPIO_CRL_MODE0 + GPIO_CRL_CNF0) << ((this->PINx - 8) * 4));
		temp = this->GPIOx->CRH;
		offset = 8;
	}
	/* output and AF mode */
	if(mode == OUTPUT || mode == AF){
			temp |= (speed << ((this->PINx - offset) * 4)); // output speed
		if(mode == AF){ // AF PP or OD mode 
			if(type == PP){ // PP 
				temp |= GPIO_CRL_CNF0_1 << ((this->PINx - offset) * 4);
			} else { // OD
				temp |= GPIO_CRL_CNF0 << ((this->PINx - offset) * 4);
			}
		} else { // General output PP or OD mode
			if(type == OD){
				temp |= GPIO_CRL_CNF0_0 << ((this->PINx - offset) * 4);
			}		
		}
	} else {
		/* input mode */	
		if(type == PP){ // input PP 
			if(pull == UP){ // pull up
				temp |= GPIO_CRL_CNF0_1 << ((this->PINx - offset) * 4);
				this->set();
			} else if (pull == DOWN) {
				temp |= GPIO_CRL_CNF0_1 << ((this->PINx - offset) * 4);
				this->reset(); // pull down
			} else { // no pull floating
				temp |= GPIO_CRL_CNF0_0 << ((this->PINx - offset) * 4);
			} // analog mode not change
		}
	}	
	/* write config to CRx reg */
	if(this->PINx < 8){
		this->GPIOx->CRL = temp;
	} else {
		this->GPIOx->CRH = temp;
	}
	/* EXTI mode config */
	if(mode == EXT_INT_RISING || mode == EXT_INT_FALLING || mode == EXT_INT_BOTH){
		/* remap pin to Alternative function External Interrup */
		uint8_t gpioIndex = ((this->GPIOx == GPIOA) ? 0U : ((this->GPIOx == GPIOB) ? 1U : ((this->GPIOx == GPIOC) ? 2U : 3U)));
		__HAL_RCC_AFIO_CLK_ENABLE();
		temp = AFIO->EXTICR[this->PINx >> 2];
		temp &= ~(0x000000F << (gpioIndex * 4));
		temp |=  (gpioIndex << (gpioIndex * 4));
		AFIO->EXTICR[this->PINx >> 2] = temp;
		
		EXTI->IMR |= 1 << (this->PINx); // set Interrupt mask reg
		/* write edge capture bit*/
		switch(mode){
			case EXT_INT_RISING:
				EXTI->RTSR |= 1 << (this->PINx);
				break;
			case EXT_INT_FALLING:
				EXTI->FTSR |= 1 << (this->PINx);
				break;
			case EXT_INT_BOTH:
				EXTI->RTSR |= 1 << (this->PINx);
				EXTI->FTSR |= 1 << (this->PINx);
				break;
			default:
				break;
		}
	}
}

void GPIO::initOutput(OTYPE type, SPEED speed){
	this->init(OUTPUT, type, NONE, speed);
}	

void GPIO::initInput(PULL pull){
	this->init(INPUT, PP, pull, LOW);
}

void GPIO::initExti(MODE edge, PULL pull,void (*CallBack)(void)){
	this->init(edge, PP, pull, LOW);
	CallBackTable[this->PINx] = CallBack;
	extiTable[this->PINx] = true;
}

void GPIO::initAF(OTYPE type, SPEED speed){
	this->init(AF, type, NONE, speed);
}

void GPIO::set(void){
	this->GPIOx->BSRR |= 1 << this->PINx;
}

void GPIO::reset(void){
	(this->GPIOx->BSRR) |= ((1 << this->PINx) << 16); 
}

void GPIO::toggle(void){
	this->GPIOx->ODR ^= 1 << (this->PINx);
}

uint8_t GPIO::read(void){
	return ((this->GPIOx->IDR >> this->PINx) & 1);
}

bool GPIO::enableEXTI(uint8_t priority){
	if(this->mode != EXT_INT_RISING && this->mode != EXT_INT_FALLING && this->mode != EXT_INT_BOTH){
		return false;
	}
	switch(this->PINx){
		case PIN0:
			HAL_NVIC_SetPriority(EXTI0_IRQn, priority, 0);
		  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
			break;
		case PIN1:
			HAL_NVIC_SetPriority(EXTI1_IRQn, priority, 0);
			HAL_NVIC_EnableIRQ(EXTI1_IRQn);
			break;
		case PIN2:
			HAL_NVIC_SetPriority(EXTI2_IRQn, priority, 0);
			HAL_NVIC_EnableIRQ(EXTI2_IRQn);
			break;
		case PIN3:
			HAL_NVIC_SetPriority(EXTI3_IRQn, priority, 0);
			HAL_NVIC_EnableIRQ(EXTI3_IRQn);
		  break;
		case PIN4:
			HAL_NVIC_SetPriority(EXTI4_IRQn, priority, 0);
			HAL_NVIC_EnableIRQ(EXTI4_IRQn);
			break;
		case PIN5:
		case PIN6:
		case PIN7:
		case PIN8:	
		case PIN9:
			HAL_NVIC_SetPriority(EXTI9_5_IRQn, priority, 0);
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			break;
		default:
			HAL_NVIC_SetPriority(EXTI15_10_IRQn, priority, 0);
			HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);						
			break;
	}
	return true;
}

bool GPIO::disableEXTI(void){
	if(this->mode != EXT_INT_RISING || this->mode != EXT_INT_FALLING || this->mode != EXT_INT_BOTH){
		return false;
	}
	switch(this->PINx){
		case PIN0:
		  HAL_NVIC_DisableIRQ(EXTI0_IRQn);
		case PIN1:
			HAL_NVIC_DisableIRQ(EXTI1_IRQn);
		case PIN2:
			HAL_NVIC_DisableIRQ(EXTI2_IRQn);
		case PIN3:
			HAL_NVIC_DisableIRQ(EXTI3_IRQn);
		case PIN4:
			HAL_NVIC_DisableIRQ(EXTI4_IRQn);
			break;
		case PIN5:
		case PIN6:
		case PIN7:
		case PIN8:	
		case PIN9:
			HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
			break;
		default:
			HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);						
			break;
	}
	return true;
}

bool GPIO::lock(void){
	__IO uint32_t setLCKK = 0; 
	__IO uint32_t resetLCKK = 0;
	setLCKK |= (1 << 16);
	setLCKK |= (1 << this->PINx);
	resetLCKK |= (1 << this->PINx);
	this->GPIOx->LCKR = setLCKK;  // Set LCKx bit(s): LCKK='1' + LCK[15-0] 
	this->GPIOx->LCKR = resetLCKK; // Reset LCKx bit(s): LCKK='0' + LCK[15-0] 
	this->GPIOx->LCKR = setLCKK;  // Set LCKx bit(s): LCKK='1' + LCK[15-0] 
	setLCKK = this->GPIOx->LCKR; // read LCKK
	if((this->GPIOx->LCKR & GPIO_LCKR_LCKK) == 1){ // checking LCKK bit
		return true;
	} else {
		return false;
	}
}

extern "C" {

void EXTI0_IRQHandler(void){
	if(extiTable[0] == true){
		CallBackTable[0]();
		EXTI->PR |= (1 << 0);
	}
}

void EXTI1_IRQHandler(void){
	if(extiTable[1] == true){
		CallBackTable[1]();
		EXTI->PR |= (1 << 1);
	}
}

void EXTI2_IRQHandler(void){
	if(extiTable[2] == true){
		CallBackTable[2]();
		EXTI->PR |= (1 << 2);
	}
}

void EXTI3_IRQHandler(void){
	if(extiTable[3] == true){
		CallBackTable[3]();
		EXTI->PR |= (1 << 3);
	}
}

void EXTI4_IRQHandler(void){
	if(extiTable[4] == true){
		CallBackTable[4]();
		EXTI->PR |= (1 << 4);
	}
}

void EXTI9_5_IRQHandler(void){
  uint8_t i;
	for(i = 5; i < 10; i++){
		if(extiTable[i] == true){
			CallBackTable[i]();
			EXTI->PR |= (1 << i);
		}
	}
}

void EXTI15_10_IRQHandler(void){
  uint8_t i;
	for(i = 10; i < 16; i++){
		if(extiTable[i] == true){
			CallBackTable[i]();
			EXTI->PR |= (1 << i);
		}
	}
}
} /* end extern C */

GPIO::~GPIO(void){
}

} /* namespace hv_driver */
