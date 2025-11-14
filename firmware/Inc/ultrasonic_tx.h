/**
 ******************************************************************************
 * @file    ultrasonic_tx.h
 * @brief   Ultrasonic transmitter driver for 40kHz transducer
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef ULTRASONIC_TX_H
#define ULTRASONIC_TX_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"

/* Timer Configuration */
#define TX_TIMER_FREQ             480000000U   // Timer clock frequency (480 MHz)
#define TX_PWM_FREQ               40000U       // 40 kHz carrier
#define TX_PWM_PERIOD             (TX_TIMER_FREQ / TX_PWM_FREQ)

/* Function Prototypes */

/**
 * @brief Initialize ultrasonic transmitter
 * @param htim Timer handle for PWM generation
 * @return HAL status
 */
HAL_StatusTypeDef ultrasonic_tx_init(TIM_HandleTypeDef *htim);

/**
 * @brief Transmit chirp signal
 * @param htim Timer handle
 * @param chirp_data Chirp signal data (normalized -1 to 1)
 * @param length Chirp length in samples
 * @return HAL status
 */
HAL_StatusTypeDef ultrasonic_tx_transmit(TIM_HandleTypeDef *htim, 
                                         float32_t *chirp_data, 
                                         uint32_t length);

/**
 * @brief Stop transmission
 * @param htim Timer handle
 * @return HAL status
 */
HAL_StatusTypeDef ultrasonic_tx_stop(TIM_HandleTypeDef *htim);

#endif /* ULTRASONIC_TX_H */

