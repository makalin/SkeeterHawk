/**
 ******************************************************************************
 * @file    stm32h7xx_it.c
 * @brief   Interrupt Service Routines
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "stm32h7xx_it.h"
#include "main.h"
#include "dfsdm_mic.h"
#include "ultrasonic_tx.h"

extern DFSDM_Filter_HandleTypeDef hdfsdm_filter[NUM_MICS];
extern TIM_HandleTypeDef htim_tx;

/**
 * @brief This function handles DFSDM interrupt.
 */
void DFSDM1_Filter0_IRQHandler(void)
{
    HAL_DFSDM_FilterIRQHandler(&hdfsdm_filter[0]);
}

void DFSDM1_Filter1_IRQHandler(void)
{
    HAL_DFSDM_FilterIRQHandler(&hdfsdm_filter[1]);
}

void DFSDM2_Filter0_IRQHandler(void)
{
    HAL_DFSDM_FilterIRQHandler(&hdfsdm_filter[2]);
}

void DFSDM2_Filter1_IRQHandler(void)
{
    HAL_DFSDM_FilterIRQHandler(&hdfsdm_filter[3]);
}

/**
 * @brief This function handles TIM1 update interrupt.
 */
void TIM1_UP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim_tx);
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

