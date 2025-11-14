cmake_minimum_required(VERSION 3.20)
project(SkeeterHawk C CXX ASM)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# STM32H7 specific settings
set(MCU_FAMILY STM32H7)
set(MCU_MODEL STM32H743xx)

# Toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Include directories
include_directories(
    Inc
    Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
)

# Source files
set(SOURCES
    Src/main.c
    Src/sonar.c
    Src/dfsdm_mic.c
    Src/ultrasonic_tx.c
    Src/guidance.c
    Src/calibration.c
    Src/data_logger.c
    Src/signal_utils.c
    Src/config.c
    Src/stm32h7xx_it.c
    Src/system_stm32h7xx.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
    Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_f32.c
    Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_correlate_f32.c
    Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c
    Drivers/CMSIS/DSP/Source/FastMathFunctions/arm_cos_f32.c
    Drivers/CMSIS/DSP/Source/FastMathFunctions/arm_sin_f32.c
    Drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_scale_f32.c
    Drivers/CMSIS/DSP/Source/StatisticsFunctions/arm_max_f32.c
)

# Compiler flags
add_compile_options(
    -mcpu=cortex-m7
    -mthumb
    -mfloat-abi=hard
    -mfpu=fpv5-d16
    -Wall
    -Wextra
    -Wno-unused-parameter
    -ffunction-sections
    -fdata-sections
    -DSTM32H743xx
    -DARM_MATH_CM7
    -D__FPU_PRESENT=1
    -DARM_MATH_MATRIX_CHECK
    -DARM_MATH_ROUNDING
)

# Linker flags
add_link_options(
    -mcpu=cortex-m7
    -mthumb
    -mfloat-abi=hard
    -mfpu=fpv5-d16
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32H743VITx_FLASH.ld
)

# Create executable
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Set output format
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    SUFFIX ".elf"
)

