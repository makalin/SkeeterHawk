# SkeeterHawk Tools and Utilities

Comprehensive guide to all tools and utility functions available in the SkeeterHawk project.

## Simulation Tools

### `analysis_tools.py`

Advanced analysis and visualization utilities for the Python simulation.

#### Functions:

1. **`calculate_snr(received_signal, signal_window, noise_window)`**
   - Calculate Signal-to-Noise Ratio in dB
   - Separates signal and noise regions for accurate SNR measurement

2. **`plot_beam_pattern(simulator, ...)`**
   - Visualize angular response (beam pattern) of the microphone array
   - Generates 2D heatmap showing beam power vs. azimuth/elevation
   - Output: `beam_pattern.png`

3. **`range_doppler_analysis(simulator, ...)`**
   - Perform Range-Doppler analysis for moving targets
   - Uses multiple pulses to extract Doppler information
   - Output: `range_doppler.png`

4. **`parameter_sweep(simulator, param_name, param_values, ...)`**
   - Sweep a parameter and measure detection performance
   - Parameters: `chirp_duration`, `chirp_bandwidth`, `noise_power`
   - Output: Parameter vs. detection error plot

5. **`plot_array_geometry(mic_positions, target_pos)`**
   - Visualize microphone array geometry
   - Shows top view (XY) and side view (XZ)
   - Output: `array_geometry.png`

6. **`analyze_detection_performance(simulator, ...)`**
   - Comprehensive performance analysis across multiple trials
   - Metrics: detection rate, range RMSE, angle RMSE, SNR
   - Output: `detection_performance.png` with 4 subplots

### `test_suite.py`

Automated test suite for validating sonar algorithms.

#### Tests:

- **Chirp Generation**: Validates LFM chirp properties
- **Matched Filter**: Tests pulse compression accuracy
- **Beamforming**: Verifies spatial filtering
- **Target Detection**: Validates detection accuracy
- **SNR Calculation**: Tests signal quality metrics
- **Multi-Target**: Tests multiple target scenarios

#### Usage:
```bash
cd simulation
python test_suite.py
```

## Firmware Utilities

### Calibration (`calibration.h/c`)

System calibration and diagnostic utilities.

#### Features:

- **Microphone Calibration**: Gain, phase, and DC offset correction
- **Temperature Compensation**: Speed of sound calibration based on temperature
- **System Diagnostics**: Signal power, noise floor, and SNR analysis per channel
- **Non-volatile Storage**: Save/load calibration data

#### Key Functions:

```c
// Initialize calibration system
calibration_init(&cal);

// Calibrate microphones
calibration_calibrate_mics(&cal, reference_signal, length);

// Set temperature for speed of sound
calibration_set_temperature(&cal, 25.0f);  // 25°C

// Apply calibration corrections
calibration_apply(&cal, raw_signals, calibrated_signals, length);

// Run diagnostics
calibration_run_diagnostics(&cal, rx_signals, length, &diag);
```

### Data Logger (`data_logger.h/c`)

Data logging system for debugging and analysis.

#### Features:

- **Circular Buffer**: Efficient logging with configurable buffer size
- **Multiple Entry Types**: Sonar data, target detections, guidance commands, vehicle state
- **UART Export**: Export logs via UART for real-time debugging
- **Timestamped Entries**: All entries include millisecond timestamps

#### Key Functions:

```c
// Initialize logger
logger_init(&logger);

// Enable logging
logger_set_enabled(&logger, true);

// Log target detection
logger_log_target(&logger, &target, confidence);

// Log guidance command
logger_log_guidance(&logger, &cmd, &vehicle_state);

// Export to UART
logger_export_uart(&logger);
```

### Signal Utilities (`signal_utils.h/c`)

Advanced signal processing utilities.

#### Features:

- **Adaptive Thresholding**: N-sigma threshold based on signal statistics
- **Peak Detection**: Find local maxima above threshold
- **Multi-Target Detection**: Cluster nearby detections and identify multiple targets
- **Bandpass Filtering**: Frequency domain filtering
- **Signal Statistics**: Mean, standard deviation, peak calculation
- **Normalization**: Signal normalization to [-1, 1] range
- **Windowing**: Apply Hanning window to reduce sidelobes

#### Key Functions:

```c
// Adaptive threshold
signal_adaptive_threshold(signal, length, &threshold);

// Find peaks
signal_find_peaks(signal, length, threshold, peaks, max_peaks, &num_peaks);

// Multi-target detection
signal_detect_multi_target(beamformed_output, length, sample_rate, &result);

// Signal statistics
signal_calculate_stats(signal, length, &mean, &std, &peak);

// Normalize signal
signal_normalize(signal, length);
```

### Configuration Management (`config.h/c`)

System configuration management.

#### Features:

- **Default Configurations**: Sensible defaults for all parameters
- **Validation**: Parameter range checking
- **Non-volatile Storage**: Save/load configuration
- **Modular Design**: Separate sonar and guidance configurations

#### Key Functions:

```c
// Initialize with defaults
config_init(&config);

// Load from flash
config_load(&config);

// Save to flash
config_save(&config);

// Get/set sonar config
config_get_sonar(&config, &sonar_config);
config_set_sonar(&config, &sonar_config);

// Get/set guidance config
config_get_guidance(&config, &guidance_config);
config_set_guidance(&config, &guidance_config);
```

## Usage Examples

### Python Analysis Workflow

```python
from sonar_sim import SonarSimulator
from analysis_tools import *

# Create simulator
sim = SonarSimulator()

# Plot beam pattern
plot_beam_pattern(sim, target_azimuth=np.radians(15))

# Analyze performance
results = analyze_detection_performance(sim, num_trials=100)

# Parameter sweep
param_values = np.linspace(0.5e-3, 2e-3, 20)  # Chirp duration
values, errors = parameter_sweep(sim, 'chirp_duration', param_values)
```

### Firmware Integration Example

```c
#include "calibration.h"
#include "data_logger.h"
#include "signal_utils.h"
#include "config.h"

// Initialize systems
system_calibration_t cal;
calibration_init(&cal);

data_logger_t logger;
logger_init(&logger);
logger_set_enabled(&logger, true);

system_config_t config;
config_init(&config);

// In main loop:
// 1. Apply calibration
calibration_apply(&cal, raw_signals, calibrated_signals, length);

// 2. Process with signal utilities
float32_t threshold;
signal_adaptive_threshold(beamformed, length, &threshold);

multi_target_result_t multi_target;
signal_detect_multi_target(beamformed, length, sample_rate, &multi_target);

// 3. Log results
if (multi_target.valid) {
    for (uint32_t i = 0; i < multi_target.num_targets; i++) {
        target_info_t target = {
            .range_cm = multi_target.targets[i].range_cm,
            .azimuth_rad = multi_target.targets[i].azimuth_rad,
            .elevation_rad = multi_target.targets[i].elevation_rad,
            .valid = true
        };
        logger_log_target(&logger, &target, 1.0f);
    }
}
```

## Build Integration

All new utilities are automatically included in the build:

- `calibration.c` - Calibration system
- `data_logger.c` - Data logging
- `signal_utils.c` - Signal processing utilities
- `config.c` - Configuration management

The CMakeLists.txt has been updated to include these files.

## Testing

Run the test suite to validate functionality:

```bash
cd simulation
python test_suite.py
```

This will run all automated tests and report pass/fail status.

## Performance Considerations

- **Calibration**: Run once at startup or when temperature changes significantly
- **Data Logger**: Use circular buffer to prevent memory overflow
- **Signal Utilities**: Multi-target detection is computationally intensive - use sparingly
- **Configuration**: Load once at startup, save only when changed

## Future Enhancements

- [ ] Real-time calibration using known reference signals
- [ ] SD card logging for extended data collection
- [ ] Advanced filtering (Kalman, particle filters)
- [ ] Machine learning integration for target classification
- [ ] Web-based configuration interface

