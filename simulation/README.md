# SkeeterHawk Simulation

Python simulation for validating the active sonar DSP pipeline before hardware implementation.

## Overview

This simulation implements:
- **LFM Chirp Generation**: Linear Frequency Modulated transmit signal
- **Matched Filtering**: Pulse compression for SNR improvement
- **Beamforming**: Delay-and-sum spatial filtering for target localization
- **Target Detection**: Angle and range estimation

## Installation

```bash
pip install -r requirements.txt
```

Or install dependencies manually:
```bash
pip install numpy matplotlib scipy
```

## Usage

Run the simulation:
```bash
python sonar_sim.py
```

This will:
1. Generate a simulated target at 2m range, 15° azimuth, 5° elevation
2. Simulate received echoes at 4 microphones
3. Apply matched filtering and beamforming
4. Display detection results and plots

## Customization

You can modify the simulation parameters in `sonar_sim.py`:

```python
# Create simulator with custom parameters
sim = SonarSimulator(
    sample_rate=200000,      # 200 kHz
    chirp_duration=0.001,    # 1 ms
    chirp_f0=38000,         # 38 kHz start
    chirp_f1=42000,         # 42 kHz end
    speed_of_sound=343.0     # m/s
)

# Simulate target at different position
received_signals, t = sim.simulate_target_echo(
    target_range=1.5,        # 1.5 meters
    target_azimuth=np.radians(30),  # 30 degrees
    target_elevation=np.radians(10), # 10 degrees
    target_rcs=1e-6,         # Radar cross section
    noise_power=0.01         # Noise level
)
```

## Output

The simulation generates:
- Console output with detection results
- `sonar_simulation_results.png` with three plots:
  1. Received signals from 4 microphones
  2. Matched filter output (pulse compression)
  3. Beamformed output with detected range

## Algorithm Details

### Matched Filtering
The matched filter performs cross-correlation between the received signal and the time-reversed transmit chirp. This compresses the echo energy into a sharp peak, dramatically improving SNR.

### Beamforming
Delay-and-sum beamforming steers a virtual listening beam by:
1. Calculating Time Difference of Arrival (TDOA) for each microphone
2. Delaying signals based on steering direction
3. Summing delayed signals to form a directional beam

The simulation searches over azimuth and elevation angles to find the direction with maximum power.

