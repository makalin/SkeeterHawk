/**
 ******************************************************************************
 * @file    stm32h7xx_it.h
 * @brief   This file contains the headers of the interrupt handlers.
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef __STM32H7xx_IT_H
#define __STM32H7xx_IT_H

#include "stm32h7xx_hal.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void DFSDM1_Filter0_IRQHandler(void);
void DFSDM1_Filter1_IRQHandler(void);
void DFSDM2_Filter0_IRQHandler(void);
void DFSDM2_Filter1_IRQHandler(void);
void TIM1_UP_IRQHandler(void);

#endif /* __STM32H7xx_IT_H */

