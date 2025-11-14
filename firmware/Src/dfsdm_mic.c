/**
 ******************************************************************************
 * @file    dfsdm_mic.c
 * @brief   DFSDM driver implementation for Knowles SPH0641LU4H-1 microphones
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "dfsdm_mic.h"
#include "sonar.h"
#include "arm_math.h"

extern DFSDM_Channel_HandleTypeDef hdfsdm_channel[NUM_MICS];
extern DFSDM_Filter_HandleTypeDef hdfsdm_filter[NUM_MICS];

/**
 * @brief Initialize DFSDM for microphone array
 */
HAL_StatusTypeDef dfsdm_mic_init(DFSDM_Filter_HandleTypeDef *hdfsdm)
{
    HAL_StatusTypeDef status;
    
    /* Configure DFSDM filter for each microphone */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        hdfsdm[i].Instance = (i == 0) ? DFSDM1_FILTER0 :
                            (i == 1) ? DFSDM1_FILTER1 :
                            (i == 2) ? DFSDM2_FILTER0 : DFSDM2_FILTER1;
        
        hdfsdm[i].Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
        hdfsdm[i].Init.RegularParam.FastMode = ENABLE;
        hdfsdm[i].Init.RegularParam.DmaMode = ENABLE;
        hdfsdm[i].Init.InjectedParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
        hdfsdm[i].Init.InjectedParam.ScanMode = ENABLE;
        hdfsdm[i].Init.InjectedParam.DmaMode = DISABLE;
        hdfsdm[i].Init.InjectedParam.ExtTrigger = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
        hdfsdm[i].Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
        hdfsdm[i].Init.InjectedParam.InjectedTrigger = DFSDM_FILTER_SW_INJECTED_TRIGGER;
        hdfsdm[i].Init.InjectedParam.InjectedContinuousMode = DISABLE;
        hdfsdm[i].Init.InjectedParam.InjectedDiscontinuousMode = DISABLE;
        hdfsdm[i].Init.InjectedParam.InjectedChannel = DFSDM_CHANNEL_0;
        hdfsdm[i].Init.InjectedParam.InjectedOffset = 0;
        hdfsdm[i].Init.FilterParam.SincOrder = DFSDM_FILTER_FAST_SINC_ORDER_3;
        hdfsdm[i].Init.FilterParam.Oversampling = DFSDM_FILTER_OVERSAMPLING;
        hdfsdm[i].Init.FilterParam.IntOversampling = 1;
        
        status = HAL_DFSDM_FilterInit(&hdfsdm[i]);
        if (status != HAL_OK) {
            return status;
        }
        
        /* Configure DFSDM channel */
        hdfsdm_channel[i].Instance = (i == 0) ? DFSDM1_CHANNEL0 :
                                    (i == 1) ? DFSDM1_CHANNEL1 :
                                    (i == 2) ? DFSDM2_CHANNEL0 : DFSDM2_CHANNEL1;
        
        hdfsdm_channel[i].Init.OutputClock.Activation = ENABLE;
        hdfsdm_channel[i].Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
        hdfsdm_channel[i].Init.OutputClock.Divider = DFSDM_CLOCK_DIV;
        hdfsdm_channel[i].Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
        hdfsdm_channel[i].Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
        hdfsdm_channel[i].Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
        hdfsdm_channel[i].Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
        hdfsdm_channel[i].Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
        hdfsdm_channel[i].Init.Awd.FilterOrder = DFSDM_CHANNEL_FAST_OVERSAMPLING_RATIO_32;
        hdfsdm_channel[i].Init.Awd.Oversampling = 1;
        hdfsdm_channel[i].Init.Offset = 0;
        hdfsdm_channel[i].Init.RightBitShift = 0;
        
        status = HAL_DFSDM_ChannelInit(&hdfsdm_channel[i]);
        if (status != HAL_OK) {
            return status;
        }
    }
    
    return HAL_OK;
}

/**
 * @brief Start microphone data acquisition
 */
HAL_StatusTypeDef dfsdm_mic_start(DFSDM_Filter_HandleTypeDef *hdfsdm, 
                                   int16_t *buffer, uint32_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Start regular conversion for all filters */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        status = HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm[i], 
                                                   &buffer[i * size], 
                                                   size);
        if (status != HAL_OK) {
            return status;
        }
    }
    
    return HAL_OK;
}

/**
 * @brief Stop microphone data acquisition
 */
HAL_StatusTypeDef dfsdm_mic_stop(DFSDM_Filter_HandleTypeDef *hdfsdm)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        status = HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm[i]);
        if (status != HAL_OK) {
            return status;
        }
    }
    
    return HAL_OK;
}

/**
 * @brief Convert PDM data to PCM
 */
void dfsdm_convert_pdm_to_pcm(int16_t *pdm_data, float32_t *pcm_data, uint32_t length)
{
    /* Convert from 16-bit integer to float32 */
    for (uint32_t i = 0; i < length; i++) {
        pcm_data[i] = (float32_t)pdm_data[i] / 32768.0f;
    }
}

