/**
 ******************************************************************************
 * @file    dfsdm_mic.h
 * @brief   DFSDM driver for Knowles SPH0641LU4H-1 microphones
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef DFSDM_MIC_H
#define DFSDM_MIC_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"

/* DFSDM Configuration */
#define DFSDM_CLOCK_DIV           4U           // Clock divider
#define DFSDM_SAMPLE_RATE         200000U      // 200 kHz
#define DFSDM_FILTER_ORDER        DFSDM_FILTER_FAST_SINC_ORDER_3
#define DFSDM_FILTER_OVERSAMPLING 64U

/* Microphone Channels */
#define DFSDM_CHANNEL_0           DFSDM_CHANNEL_0
#define DFSDM_CHANNEL_1           DFSDM_CHANNEL_1
#define DFSDM_CHANNEL_2           DFSDM_CHANNEL_2
#define DFSDM_CHANNEL_3           DFSDM_CHANNEL_3

/* Function Prototypes */

/**
 * @brief Initialize DFSDM for microphone array
 * @param hdfsdm DFSDM handle
 * @return HAL status
 */
HAL_StatusTypeDef dfsdm_mic_init(DFSDM_Filter_HandleTypeDef *hdfsdm);

/**
 * @brief Start microphone data acquisition
 * @param hdfsdm DFSDM handle
 * @param buffer Buffer for received data
 * @param size Buffer size in samples
 * @return HAL status
 */
HAL_StatusTypeDef dfsdm_mic_start(DFSDM_Filter_HandleTypeDef *hdfsdm, 
                                   int16_t *buffer, uint32_t size);

/**
 * @brief Stop microphone data acquisition
 * @param hdfsdm DFSDM handle
 * @return HAL status
 */
HAL_StatusTypeDef dfsdm_mic_stop(DFSDM_Filter_HandleTypeDef *hdfsdm);

/**
 * @brief Convert PDM data to PCM
 * @param pdm_data PDM input data
 * @param pcm_data PCM output data
 * @param length Number of samples
 */
void dfsdm_convert_pdm_to_pcm(int16_t *pdm_data, float32_t *pcm_data, uint32_t length);

#endif /* DFSDM_MIC_H */

