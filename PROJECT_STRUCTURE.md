# SkeeterHawk Project Structure

Complete source code structure for the SkeeterHawk autonomous acoustic mosquito interceptor.

## Directory Structure

```
SkeeterHawk/
├── README.md                 # Main project documentation
├── LICENSE                   # MIT License
├── Makefile                  # Build automation
├── .gitignore               # Git ignore rules
│
├── simulation/               # Python simulation
│   ├── README.md            # Simulation documentation
│   ├── requirements.txt     # Python dependencies
│   └── sonar_sim.py         # Main simulation script
│
├── firmware/                 # STM32H7 embedded firmware
│   ├── README.md            # Firmware documentation
│   ├── CMakeLists.txt       # CMake build configuration
│   ├── Inc/                 # Header files
│   │   ├── main.h           # Main application header
│   │   ├── sonar.h          # Sonar processing API
│   │   ├── dfsdm_mic.h      # Microphone driver
│   │   ├── ultrasonic_tx.h  # Transmitter driver
│   │   ├── guidance.h       # Guidance law
│   │   └── stm32h7xx_it.h   # Interrupt handlers
│   └── Src/                 # Source files
│       ├── main.c           # Main application loop
│       ├── sonar.c          # Sonar processing implementation
│       ├── dfsdm_mic.c      # Microphone driver
│       ├── ultrasonic_tx.c  # Transmitter driver
│       ├── guidance.c       # Guidance law implementation
│       ├── stm32h7xx_it.c   # Interrupt service routines
│       └── system_stm32h7xx.c # System initialization
│
└── hardware/                 # Hardware design
    └── README.md            # Hardware documentation
```

## Key Components

### 1. Simulation (`simulation/`)
- **Purpose**: Validate DSP algorithms before hardware implementation
- **Language**: Python 3
- **Dependencies**: NumPy, SciPy, Matplotlib
- **Features**:
  - LFM chirp generation
  - Matched filtering (pulse compression)
  - Delay-and-sum beamforming
  - Target detection and localization

### 2. Firmware (`firmware/`)
- **Target**: STM32H743VIT6 (Cortex-M7 @ 480MHz)
- **Language**: C (C11 standard)
- **Libraries**: STM32 HAL, CMSIS DSP
- **Modules**:
  - **Sonar Processing**: Real-time matched filtering and beamforming
  - **DFSDM Driver**: Interface to 4x PDM microphones
  - **Ultrasonic TX**: 40kHz LFM chirp generation via PWM
  - **Guidance**: Proportional navigation for target intercept
  - **Main Loop**: Integration of all subsystems

### 3. Hardware (`hardware/`)
- **Documentation**: Component specifications and PCB design notes
- **Key Components**:
  - STM32H7 MCU
  - 4x Knowles SPH0641LU4H-1 microphones
  - 40kHz ultrasonic transducer
  - H-bridge driver

## Build Instructions

### Simulation
```bash
cd simulation
pip install -r requirements.txt
python sonar_sim.py
```

### Firmware
```bash
cd firmware
mkdir build && cd build
cmake ..
make
```

Or use the top-level Makefile:
```bash
make install-deps  # Install Python dependencies
make sim           # Run simulation
make firmware      # Build firmware (requires toolchain)
```

## Algorithm Overview

### Signal Processing Pipeline

1. **Transmit**: Generate and emit 40kHz LFM chirp (38-42kHz, 1ms duration)
2. **Receive**: Acquire signals from 4-microphone array at 200kHz
3. **Matched Filter**: Cross-correlate received signals with time-reversed chirp
4. **Beamforming**: Delay-and-sum spatial filtering to localize target
5. **Detection**: Find peak in beamformed output to estimate range and angle

### Guidance Law

Proportional Navigation (PN) guidance:
- Navigation constant: N = 3.0
- Computes acceleration commands based on line-of-sight rate
- Converts to motor thrust commands for quadcopter control

## Development Status

- [x] **Phase 0**: Feasibility Study (Python simulation)
- [ ] **Phase 1**: Grounded Sonar Rig (Hardware validation)
- [ ] **Phase 2**: SWaP Integration (Custom PCB)
- [ ] **Phase 3**: Closed-Loop Intercept (Flight testing)

## Notes

- The firmware requires STM32CubeH7 HAL drivers (not included)
- Hardware-specific configurations (GPIO, DFSDM, TIM) need to be customized
- Motor control interface depends on flight controller choice
- Vehicle state estimation (IMU) integration required for closed-loop control

