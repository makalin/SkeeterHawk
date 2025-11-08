# SkeeterHawk 🦟❌

> **Autonomous Acoustic Mosquito Interceptor**
>
> *Turning 40-gram micro-drones into robotic bats using active sonar beamforming.*

[![Status](https://img.shields.io/badge/Status-Pre--Alpha-red)]()
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-STM32H7-green)]()

## 🎯 Mission
SkeeterHawk is an advanced embedded engineering initiative with a seemingly simple goal: **detect, intercept, and neutralize mosquitoes autonomously indoors.**

The challenge is extreme Size, Weight, and Power (SWaP) constraints. A mosquito's acoustic signature (wing beat) is too faint (~35dB SPL) to detect passively over the noise of drone propellers (~80dB SPL).

**SkeeterHawk solves this by mimicking nature:** instead of passive listening, we implement **active ultrasonic echolocation** (sonar) on a sub-40g micro-aerial vehicle.

## 🦇 System Concept

We are effectively building a flying phased-array sonar. The system does not rely on cameras (too heavy, poor low-light performance, high latency). It relies entirely on standard, low-cost acoustic components pushed to their absolute limit via Digital Signal Processing (DSP).

1.  **The Shout (Tx):** A 40kHz ultrasonic transducer emits a Linear Frequency Modulated (LFM) chirp.
2.  **The Ears (Rx):** A 2x2 array of MEMS smartphone microphones captures raw audio at >100ksps.
3.  **The Brain (DSP):** An onboard STM32H7 performs real-time Matched Filtering and Beamforming to localize targets with <5cm accuracy.
4.  **The Intercept (GNC):** Proportional Navigation algorithms guide the drone to collide with the target using its propellers as the kill mechanism.

## 🛠️ Hardware Stack

The platform is designed around extreme weight savings. Every gram counts.

| Component | Specification | Purpose |
| :--- | :--- | :--- |
| **Flight Platform** | Custom or heavily modified Bitcraze Crazyflie 2.1 | Base avionics and propulsion (<27g) |
| **MCU** | **STM32H743** (Cortex-M7 @ 480MHz) | Real-time FFTs, DFSDM for mics, flight control |
| **Microphones** | 4x **Knowles SPH0641LU4H-1** | Digital PDM, ultrasonic mode enabled (up to 80kHz) |
| **Emitter** | 40kHz Automotive Transducer (Stripped) | High-power active sonar pulse generation |
| **Driver** | Minimalist H-Bridge | Driving the emitter with ~12V p-p from 1S LiPo |

## 🧠 The Signal Processing Pipeline

Achieving detection of a 3mm target at 2 meters requires significant **Processing Gain** to pull the signal out of the propeller noise floor.

### 1. Pulse Compression (Matched Filter)
We do not transmit a simple "ping." We transmit a chirp $s_{tx}(t)$ sweeping 38kHz–42kHz. Received signals are convolved with the time-reversed transmit chirp:
$$y[n] = \sum (r[k] \cdot s_{tx}[n-k])$$
This compresses 1ms of low-intensity reflected energy into a single, sharp high-intensity peak, drastically improving Signal-to-Noise Ratio (SNR).

### 2. Delay-and-Sum Beamforming
To determine the mosquito's azimuth and elevation, we spatially filter the 4 microphone channels. By artificially delaying signals based on their Time Difference of Arrival (TDOA), we "steer" a virtual listening beam:
$$Beam(\theta, \phi) = \sum_{i=1}^{4} w_i \cdot y_i(t - \tau_i(\theta, \phi))$$

## 🗺️ Roadmap

- [x] **Phase 0: Feasibility Study** (Physics simulation & DSP math validation)
- [ ] **Phase 1: Grounded Sonar Rig** (Benchtop validation using Teensy 4.1 + Mic Array)
- [ ] **Phase 2: SWaP Integration** (Custom PCB design with STM32H7)
- [ ] **Phase 3: Closed-Loop Intercept** (Flight controller integration with Proportional Navigation)

## 💻 Development & Simulation

Before building hardware, you can validate the DSP concepts using our Python simulator.

```bash
# Clone the repo
git clone [https://github.com/makalin/SkeeterHawk.git](https://github.com/makalin/SkeeterHawk.git)
cd SkeeterHawk/simulation

# Install dependencies
pip install numpy matplotlib scipy

# Run the matched filter active sonar simulation
python sonar_sim.py
````

## 🤝 Contributing

This project requires expertise in:

  * High-speed embedded systems (STM32H7, DMA, RTOS)
  * Acoustic DSP (Beamforming, filtering)
  * Control Theory (Drone stabilization, guidance laws)
  * PCB Design (High-frequency mixed-signal)

Check the [Issues](https://github.com/makalin/SkeeterHawk/issues) tab for current tasks.

## 📄 License

Copyright © 2025 Mehmet T. AKALIN.
This project is MIT licensed. See `LICENSE` for more information.
