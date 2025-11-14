/**
 ******************************************************************************
 * @file    system_stm32h7xx.c
 * @brief   CMSIS Cortex-M7 Device Peripheral Access Layer System Source File
 * @author  STMicroelectronics
 * @date    2025
 ******************************************************************************
 */

#include "stm32h7xx.h"

/**
 * @brief  Setup the microcontroller system
 *         Initialize the FPU setting, vector table location and the PLL configuration
 */
void SystemInit(void)
{
    /* FPU settings */
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
        SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
    #endif
    
    /* Reset the RCC clock configuration to the default reset state */
    /* Set HSION bit */
    RCC->CR |= RCC_CR_HSION;
    
    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;
    
    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= 0xFEF6FFFF;
    
    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x00000000;
    
    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFF;
    
    /* Disable all interrupts */
    RCC->CIR = 0x00000000;
    
    /* Configure the Vector Table location add offset address */
    #ifdef VECT_TAB_SRAM
        SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;
    #else
        SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
    #endif
}

