# SkeeterHawk Firmware

STM32H7 embedded firmware for the SkeeterHawk autonomous mosquito interceptor.

## Overview

This firmware implements:
- **Active Sonar Processing**: Matched filtering and beamforming for target detection
- **DFSDM Driver**: Interface to Knowles SPH0641LU4H-1 MEMS microphones
- **Ultrasonic Transmitter**: 40kHz LFM chirp generation
- **Proportional Navigation**: Guidance law for target intercept

## Building

### Prerequisites

- ARM GCC toolchain (arm-none-eabi-gcc)
- CMake 3.20 or later
- STM32CubeH7 HAL drivers (should be placed in `Drivers/` directory)
- CMSIS DSP library (included in STM32CubeH7)

### Build Steps

```bash
cd firmware
mkdir build
cd build
cmake ..
make
```

The output will be `SkeeterHawk.elf` which can be flashed to the STM32H743.

## Project Structure

```
firmware/
├── Inc/           # Header files
│   ├── sonar.h           # Sonar processing API
│   ├── dfsdm_mic.h       # Microphone driver
│   ├── ultrasonic_tx.h  # Transmitter driver
│   ├── guidance.h        # Guidance law
│   └── main.h            # Main application
├── Src/           # Source files
│   ├── main.c            # Main application loop
│   ├── sonar.c           # Sonar processing implementation
│   ├── dfsdm_mic.c       # Microphone driver implementation
│   ├── ultrasonic_tx.c   # Transmitter implementation
│   ├── guidance.c        # Guidance law implementation
│   ├── stm32h7xx_it.c    # Interrupt handlers
│   └── system_stm32h7xx.c # System initialization
└── CMakeLists.txt # Build configuration
```

## Hardware Requirements

- STM32H743VIT6 MCU
- 4x Knowles SPH0641LU4H-1 MEMS microphones (PDM interface)
- 40kHz ultrasonic transducer with H-bridge driver
- Flight controller interface (e.g., Crazyflie 2.1 base)

## Configuration

Key parameters can be adjusted in `Inc/sonar.h`:
- `SONAR_SAMPLE_RATE`: Sampling rate (default: 200 kHz)
- `SONAR_CHIRP_DURATION_MS`: Chirp duration (default: 1 ms)
- `SONAR_CHIRP_F0/F1`: Chirp frequency range (default: 38-42 kHz)
- `DETECTION_THRESHOLD`: Minimum detection amplitude

## Usage

The main loop performs the following sequence:
1. Transmit LFM chirp
2. Acquire microphone data via DFSDM
3. Apply matched filtering to each channel
4. Perform beamforming to detect target
5. Compute guidance command using proportional navigation
6. Convert to motor commands

## Notes

- The DFSDM and TIM configurations in `main.c` are placeholders and need to be configured based on your specific hardware setup
- Motor control interface needs to be implemented based on your flight controller
- Vehicle state estimation (IMU/GPS) integration is required for closed-loop control

