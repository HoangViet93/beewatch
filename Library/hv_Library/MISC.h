/**
  ******************************************************************************
 * @file    MISC.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    6-12-2015
 * @brief   some misc funtion
  */
//-------------------------------------------------------------------------

#ifndef MISC_H
#define MISC_H

#include "stm32f1xx.h"

namespace hv_driver {
	
void Sys_init(void);
void Sys_subISRReset(void);
bool Sys_subISRAssign(void (* pSubISR)(void));
bool Sys_subISRRemove(void (* pSubISR)(void));
uint32_t Sys_getTick(void);
void Sys_Delayms(__IO uint16_t time_ms);
void Sys_timSystickInit(void);
void SystemClock_Config(void);	

} /* hv_driver */

#endif /* MISC_H */