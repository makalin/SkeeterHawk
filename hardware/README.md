# SkeeterHawk Hardware

Hardware design files and documentation for the SkeeterHawk autonomous mosquito interceptor.

## Hardware Stack

| Component | Specification | Purpose |
| :--- | :--- | :--- |
| **Flight Platform** | Custom or heavily modified Bitcraze Crazyflie 2.1 | Base avionics and propulsion (<27g) |
| **MCU** | **STM32H743VIT6** (Cortex-M7 @ 480MHz) | Real-time FFTs, DFSDM for mics, flight control |
| **Microphones** | 4x **Knowles SPH0641LU4H-1** | Digital PDM, ultrasonic mode enabled (up to 80kHz) |
| **Emitter** | 40kHz Automotive Transducer (Stripped) | High-power active sonar pulse generation |
| **Driver** | Minimalist H-Bridge | Driving the emitter with ~12V p-p from 1S LiPo |

## PCB Design Considerations

### Weight Optimization
- Every gram counts - use 4-layer board with minimal via count
- Remove unnecessary components and use smallest package sizes
- Consider removing connectors where possible (direct solder)

### Signal Integrity
- Keep microphone traces short and matched length
- Separate analog and digital grounds
- Proper decoupling for DFSDM clock domains
- Shield ultrasonic transducer driver from sensitive analog sections

### Power Management
- Efficient LDOs for 3.3V and 5V rails
- H-bridge driver with minimal quiescent current
- Power gating for unused peripherals

## Microphone Array Layout

```
        [Mic 2]  [Mic 3]
           |        |
           |  10mm  |
           |        |
        [Mic 0]  [Mic 1]
```

- 2x2 array with 10mm spacing
- Optimized for beamforming at 40kHz (wavelength ~8.6mm)
- All microphones on same plane for 2D localization

## Ultrasonic Transmitter

- 40kHz center frequency
- LFM chirp: 38kHz to 42kHz
- H-bridge driver for high-power output
- Stripped automotive transducer for weight savings

## Integration Notes

- STM32H7 DFSDM channels: Use DFSDM1_CH0-1 and DFSDM2_CH0-1
- Timer for PWM: TIM1 or TIM8 for ultrasonic transmitter
- Flight controller interface: UART or SPI for motor commands
- IMU interface: SPI for vehicle state estimation

## Future Improvements

- [ ] Custom PCB design with integrated components
- [ ] Optimized antenna pattern for transducer
- [ ] Multi-frequency operation for better target discrimination
- [ ] Adaptive beamforming for moving targets

