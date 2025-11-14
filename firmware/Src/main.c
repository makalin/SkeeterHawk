/**
 ******************************************************************************
 * @file    main.c
 * @brief   Main application for SkeeterHawk
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "main.h"
#include "sonar.h"
#include "dfsdm_mic.h"
#include "ultrasonic_tx.h"
#include "guidance.h"
#include "stm32h7xx_hal.h"
#include <string.h>

/* Private variables */
sonar_state_t sonar_state;
vehicle_state_t vehicle_state;
guidance_cmd_t guidance_cmd;
float motor_thrust[4];

DFSDM_Filter_HandleTypeDef hdfsdm_filter[NUM_MICS];
DFSDM_Channel_HandleTypeDef hdfsdm_channel[NUM_MICS];
TIM_HandleTypeDef htim_tx;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DFSDM_Init(void);
static void MX_TIM_Init(void);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DFSDM_Init();
    MX_TIM_Init();
    
    /* Initialize sonar system */
    if (sonar_init(&sonar_state) != 0) {
        Error_Handler();
    }
    
    /* Initialize guidance system */
    guidance_init();
    
    /* Initialize vehicle state */
    memset(&vehicle_state, 0, sizeof(vehicle_state_t));
    
    /* Initialize DFSDM for microphones */
    if (dfsdm_mic_init(hdfsdm_filter) != HAL_OK) {
        Error_Handler();
    }
    
    /* Initialize ultrasonic transmitter */
    if (ultrasonic_tx_init(&htim_tx) != HAL_OK) {
        Error_Handler();
    }
    
    /* Main loop */
    while (1) {
        /* Transmit chirp */
        ultrasonic_tx_transmit(&htim_tx, sonar_state.tx_chirp, SONAR_CHIRP_SAMPLES);
        
        /* Wait for echo window */
        HAL_Delay(30);  // ~30ms for 5m max range
        
        /* Start microphone acquisition */
        int16_t mic_buffer[NUM_MICS * SONAR_MAX_SAMPLES];
        dfsdm_mic_start(hdfsdm_filter, mic_buffer, SONAR_MAX_SAMPLES);
        
        /* Wait for acquisition to complete */
        HAL_Delay(30);
        dfsdm_mic_stop(hdfsdm_filter);
        
        /* Convert PDM to PCM and copy to sonar buffers */
        for (uint32_t i = 0; i < NUM_MICS; i++) {
            dfsdm_convert_pdm_to_pcm(&mic_buffer[i * SONAR_MAX_SAMPLES],
                                     sonar_state.rx_buffer[i],
                                     SONAR_MAX_SAMPLES);
        }
        sonar_state.sample_count = SONAR_MAX_SAMPLES;
        
        /* Process sonar data */
        if (sonar_detect_target(&sonar_state) == 0) {
            /* Get target information */
            target_info_t target;
            if (sonar_get_target(&sonar_state, &target) == 0) {
                /* Compute guidance command */
                if (guidance_compute(&vehicle_state, &target, &guidance_cmd) == 0) {
                    /* Convert to motor commands */
                    guidance_to_motors(&guidance_cmd, motor_thrust);
                    
                    /* Apply motor commands (would interface with flight controller) */
                    // motor_set_thrust(motor_thrust);
                }
            }
        }
        
        /* Update vehicle state (would come from IMU/GPS) */
        // vehicle_state = get_vehicle_state();
        
        /* Small delay */
        HAL_Delay(10);
    }
}

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure the main internal regulator output voltage */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    /* Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    /* Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief DFSDM Initialization Function
 */
static void MX_DFSDM_Init(void)
{
    /* DFSDM initialization would go here */
    /* This is a placeholder - actual implementation depends on hardware */
}

/**
 * @brief TIM Initialization Function
 */
static void MX_TIM_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    htim_tx.Instance = TIM1;
    htim_tx.Init.Prescaler = 0;
    htim_tx.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_tx.Init.Period = TX_PWM_PERIOD - 1;
    htim_tx.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_tx.Init.RepetitionCounter = 0;
    htim_tx.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim_tx) != HAL_OK) {
        Error_Handler();
    }
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim_tx, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    
    if (HAL_TIM_PWM_ConfigChannel(&htim_tx, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIM_MasterConfigSynchronization(&htim_tx, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
    /* GPIO initialization would go here */
}

/**
 * @brief  This function is executed in case of error occurrence.
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        /* Error handler */
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number */
}
#endif

