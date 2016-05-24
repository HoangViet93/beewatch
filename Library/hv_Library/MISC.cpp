/**
  ******************************************************************************
 * @file    MISC.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    6-12-2015
 * @brief   some misc funtion
  */
//-------------------------------------------------------------------------
#include "MISC.h"

#define MAX_ISR 5

/* Local Variable */
void (*subISRTable[MAX_ISR]) (void); // Sub ISR Table function pointer array
__IO uint32_t tim4Tick = 0;
__IO USART_TypeDef* USARTx_ = USART1;

/* Local Function prototype */
void SystemClock_Config(void);	

namespace hv_driver {
	
void Sys_init(void){
	HAL_Init();
	SystemClock_Config();
	Sys_timSystickInit();
}


void SystemClock_Config(void){
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Configure PLL ------------------------------------------------------*/
  /* PLL configuration: PLLCLK = (HSI / 2) * PLLMUL = (8 / 2) * 16 = 64 MHz */
  /* PREDIV1 configuration: PREDIV1CLK = PLLCLK / HSEPredivValue = 64 / 1 = 64 MHz */
  /* Enable HSI and activate PLL with HSi_DIV2 as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSI;
  oscinitstruct.HSEState        = RCC_HSE_OFF;
  oscinitstruct.LSEState        = RCC_LSE_OFF;
  oscinitstruct.HSIState        = RCC_HSI_ON;
  oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscinitstruct.HSEPredivValue    = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSI_DIV2;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}	

/**
  * @brief  Using Timer4 as alternative Systick timer
  * @param  none
  * @return	none
  */
void Sys_timSystickInit(void){
	__HAL_RCC_TIM4_CLK_ENABLE();

	TIM_HandleTypeDef 								TimSystick;
	TimSystick.Instance								= TIM4;
	TimSystick.Init.RepetitionCounter	= 0;
	TimSystick.Init.CounterMode				= TIM_COUNTERMODE_UP;
	TimSystick.Init.Period						= 10 - 1;	// 1kHz Interrupt
	TimSystick.Init.Prescaler					= (SystemCoreClock/10000) - 1;	// 10kHz operation frequency
	TimSystick.Init.ClockDivision			= 0;
	
  /* Set the TIM4 priority */
  HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);

  /* Enable the TIM4 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
	
	HAL_TIM_Base_Init(&TimSystick);
	HAL_TIM_Base_Start_IT(&TimSystick);	// start interrupt
}

/**
  * @brief  Clear all Interrupt func in subISR Table
  * @param  none
  * @return none
  */
void Sys_subISRReset(void){
	for (uint8_t i = 0; i < MAX_ISR; i++) {
		subISRTable[i] = NULL;
	}
}

/**
  * @brief  Assign function to ISR Table
  * @param  void (* pSubISR)(void) - pointer to function
  * @return	true if available slot on Table assign success.
  */
bool Sys_subISRAssign(void (* pSubISR)(void)){
	for (uint8_t i = 0; i < MAX_ISR; i++) {
		if(subISRTable[i] == NULL) {
			subISRTable[i] = pSubISR; // assign interrupt function
			return true;
		}
	}
	return false;
}

/**
  * @brief  Remove function to ISR Table
  * @param  void (* pSubISR)(void) - pointer to function
  * @return true if found function on table remove success
  */
bool Sys_subISRRemove(void (* pSubISR)(void)){
	for (uint8_t i = 0; i < MAX_ISR; i++) {
		if(subISRTable[i] == pSubISR) {
			subISRTable[i] = NULL;
			return true;
		}
	}
	return false;
}

/**
  * @brief  ISR Handler excute all function in table
  * @param  none
  * @return	none
  */
void Sys_timISRHandler(void){
	tim4Tick++; // increase tim4 tick
	
	for (uint8_t i = 0; i < MAX_ISR; i++) {
		if (subISRTable[i] != NULL) {	
			subISRTable[i] ();
		}
	}
}

/**
  * @brief  return tim4 tick value
  * @param  none
  * @return tim4Tick
  */
uint32_t Sys_getTick(void){
	return tim4Tick;
}

/**
  * @brief  Delay milisecond function
  * @param  uint16_t time_ms - amount of time wanto delay
  * @return	none
  */
void Sys_Delayms(__IO uint16_t time_ms){
	uint32_t tickStart = 0;

	tickStart = Sys_getTick();
	while (Sys_getTick() - tickStart < time_ms);
}

extern "C" {
/**
  * @brief  TIM4 SysTick handles 
  * @param  None
  * @retval None
  */
void TIM4_IRQHandler(void) {
	if (((TIM4->SR & TIM_FLAG_UPDATE) == TIM_FLAG_UPDATE) != RESET) {	// check interrupt flag
		if ((((TIM4->DIER & TIM_IT_UPDATE) == TIM_IT_UPDATE) ? SET : RESET) != RESET) {	// check interrupt type
			TIM4->SR = ~TIM_IT_UPDATE; // clear interrupt
			Sys_timISRHandler(); // call tim Systick handler
		}
	}
}
}

} /* hv_driver */
