/**
 ******************************************************************************
 * @file    ultrasonic_tx.c
 * @brief   Ultrasonic transmitter driver implementation
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "ultrasonic_tx.h"
#include "sonar.h"
#include "arm_math.h"

/**
 * @brief Initialize ultrasonic transmitter
 */
HAL_StatusTypeDef ultrasonic_tx_init(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    
    /* Configure timer for PWM generation at 40 kHz */
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    htim->Instance->ARR = TX_PWM_PERIOD - 1;
    htim->Instance->PSC = 0;
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = TX_PWM_PERIOD / 2;  // 50% duty cycle base
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

/**
 * @brief Transmit chirp signal using PWM modulation
 */
HAL_StatusTypeDef ultrasonic_tx_transmit(TIM_HandleTypeDef *htim, 
                                         float32_t *chirp_data, 
                                         uint32_t length)
{
    if (htim == NULL || chirp_data == NULL) {
        return HAL_ERROR;
    }
    
    /* Start PWM */
    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1) != HAL_OK) {
        return HAL_ERROR;
    }
    
    /* Modulate chirp onto carrier using DMA */
    /* This would typically use a DMA transfer to update the timer compare register */
    /* For now, we'll use a simplified approach */
    
    for (uint32_t i = 0; i < length; i++) {
        /* Amplitude modulation: adjust duty cycle based on chirp amplitude */
        uint32_t pulse = (TX_PWM_PERIOD / 2) + (uint32_t)(chirp_data[i] * TX_PWM_PERIOD / 4);
        
        /* Update compare register */
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, pulse);
        
        /* Wait for next sample period */
        uint32_t sample_period_us = 1000000U / SONAR_SAMPLE_RATE;
        HAL_Delay(sample_period_us / 1000);
    }
    
    return HAL_OK;
}

/**
 * @brief Stop transmission
 */
HAL_StatusTypeDef ultrasonic_tx_stop(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    
    return HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
}

