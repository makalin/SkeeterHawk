#!/usr/bin/env python3
"""
SkeeterHawk Analysis Tools
==========================
Utility functions for analyzing sonar performance, SNR, beam patterns, etc.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
from scipy import signal
from typing import Tuple, List, Optional
import warnings
warnings.filterwarnings('ignore')


def calculate_snr(received_signal: np.ndarray, 
                  signal_window: Tuple[int, int],
                  noise_window: Tuple[int, int]) -> float:
    """
    Calculate Signal-to-Noise Ratio (SNR) in dB.
    
    Args:
        received_signal: Received signal array
        signal_window: (start, end) indices for signal region
        noise_window: (start, end) indices for noise region
        
    Returns:
        SNR in dB
    """
    signal_region = received_signal[signal_window[0]:signal_window[1]]
    noise_region = received_signal[noise_window[0]:noise_window[1]]
    
    signal_power = np.mean(signal_region ** 2)
    noise_power = np.mean(noise_region ** 2)
    
    if noise_power == 0:
        return np.inf
    
    snr_linear = signal_power / noise_power
    snr_db = 10 * np.log10(snr_linear)
    
    return snr_db


def plot_beam_pattern(simulator, 
                     azimuth_range: Tuple[float, float] = (-np.pi/2, np.pi/2),
                     elevation_range: Tuple[float, float] = (-np.pi/4, np.pi/4),
                     num_points: int = 50,
                     target_range: float = 2.0,
                     target_azimuth: float = 0.0,
                     target_elevation: float = 0.0) -> np.ndarray:
    """
    Plot beam pattern (angular response) of the array.
    
    Args:
        simulator: SonarSimulator instance
        azimuth_range: Range of azimuth angles to test
        elevation_range: Range of elevation angles to test
        num_points: Number of points in each dimension
        target_range: Range to simulated target
        target_azimuth: Azimuth of simulated target
        target_elevation: Elevation of simulated target
        
    Returns:
        2D array of beam pattern power
    """
    # Simulate target
    received_signals, t = simulator.simulate_target_echo(
        target_range, target_azimuth, target_elevation
    )
    
    # Apply matched filter
    filtered_signals = np.zeros_like(received_signals)
    for i in range(4):
        filtered_signals[i, :] = simulator.matched_filter_process(received_signals[i, :])
    
    # Scan angles
    azimuths = np.linspace(azimuth_range[0], azimuth_range[1], num_points)
    elevations = np.linspace(elevation_range[0], elevation_range[1], num_points)
    
    beam_pattern = np.zeros((num_points, num_points))
    
    for i, az in enumerate(azimuths):
        for j, el in enumerate(elevations):
            beamformed = simulator.beamform(filtered_signals, az, el)
            beam_pattern[j, i] = np.max(np.abs(beamformed))
    
    # Plot
    fig, ax = plt.subplots(figsize=(10, 8))
    im = ax.imshow(beam_pattern, 
                   extent=[np.degrees(azimuth_range[0]), np.degrees(azimuth_range[1]),
                          np.degrees(elevation_range[0]), np.degrees(elevation_range[1])],
                   aspect='auto', origin='lower', cmap='viridis')
    ax.set_xlabel('Azimuth (degrees)')
    ax.set_ylabel('Elevation (degrees)')
    ax.set_title('Beam Pattern (Angular Response)')
    ax.axvline(np.degrees(target_azimuth), color='r', linestyle='--', label='Target')
    ax.axhline(np.degrees(target_elevation), color='r', linestyle='--')
    plt.colorbar(im, label='Beam Power')
    plt.legend()
    plt.tight_layout()
    plt.savefig('beam_pattern.png', dpi=150)
    plt.show()
    
    return beam_pattern


def range_doppler_analysis(simulator,
                          target_range: float,
                          target_azimuth: float,
                          target_elevation: float,
                          target_velocity: float = 0.0,
                          num_ranges: int = 100,
                          num_dopplers: int = 50) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Perform Range-Doppler analysis for moving targets.
    
    Args:
        simulator: SonarSimulator instance
        target_range: Target range in meters
        target_azimuth: Target azimuth in radians
        target_elevation: Target elevation in radians
        target_velocity: Target radial velocity in m/s
        num_ranges: Number of range bins
        num_dopplers: Number of Doppler bins
        
    Returns:
        Tuple of (range_axis, doppler_axis, range_doppler_map)
    """
    # Simulate multiple pulses for Doppler processing
    num_pulses = 32
    pulse_repetition_interval = 0.1  # 100ms
    
    range_axis = np.linspace(0.1, 5.0, num_ranges)
    doppler_axis = np.linspace(-10, 10, num_dopplers)  # m/s
    
    range_doppler_map = np.zeros((num_dopplers, num_ranges))
    
    for pulse_idx in range(num_pulses):
        # Update target range based on velocity
        current_range = target_range + target_velocity * pulse_idx * pulse_repetition_interval
        
        # Simulate echo
        received_signals, t = simulator.simulate_target_echo(
            current_range, target_azimuth, target_elevation
        )
        
        # Process first microphone
        matched_output = simulator.matched_filter_process(received_signals[0, :])
        
        # Convert to range
        ranges = (t * simulator.c) / 2.0
        
        # Interpolate to range bins
        for i, range_bin in enumerate(range_axis):
            idx = np.argmin(np.abs(ranges - range_bin))
            if idx < len(matched_output):
                range_doppler_map[:, i] += np.abs(matched_output[idx])
    
    # Apply FFT across pulses for Doppler
    range_doppler_map = np.abs(np.fft.fftshift(np.fft.fft(range_doppler_map, axis=0, n=num_dopplers)))
    
    # Plot
    fig, ax = plt.subplots(figsize=(12, 8))
    im = ax.imshow(20 * np.log10(range_doppler_map + 1e-10),
                   extent=[range_axis[0], range_axis[-1],
                          doppler_axis[0], doppler_axis[-1]],
                   aspect='auto', origin='lower', cmap='jet')
    ax.set_xlabel('Range (m)')
    ax.set_ylabel('Doppler Velocity (m/s)')
    ax.set_title('Range-Doppler Map')
    plt.colorbar(im, label='Power (dB)')
    plt.tight_layout()
    plt.savefig('range_doppler.png', dpi=150)
    plt.show()
    
    return range_axis, doppler_axis, range_doppler_map


def parameter_sweep(simulator,
                   param_name: str,
                   param_values: np.ndarray,
                   target_range: float = 2.0,
                   target_azimuth: float = 0.0,
                   target_elevation: float = 0.0) -> Tuple[np.ndarray, np.ndarray]:
    """
    Sweep a parameter and measure detection performance.
    
    Args:
        simulator: SonarSimulator instance
        param_name: Parameter to sweep ('chirp_duration', 'chirp_bandwidth', 'noise_power')
        param_values: Array of parameter values to test
        target_range: Target range
        target_azimuth: Target azimuth
        target_elevation: Target elevation
        
    Returns:
        Tuple of (param_values, detection_errors)
    """
    detection_errors = []
    
    for param_val in param_values:
        # Create new simulator with modified parameter
        if param_name == 'chirp_duration':
            test_sim = SonarSimulator(
                sample_rate=simulator.fs,
                chirp_duration=param_val,
                chirp_f0=simulator.chirp_f0,
                chirp_f1=simulator.chirp_f1
            )
        elif param_name == 'chirp_bandwidth':
            center_freq = (simulator.chirp_f0 + simulator.chirp_f1) / 2
            test_sim = SonarSimulator(
                sample_rate=simulator.fs,
                chirp_duration=simulator.chirp_duration,
                chirp_f0=center_freq - param_val/2,
                chirp_f1=center_freq + param_val/2
            )
        else:
            test_sim = simulator
        
        # Simulate and detect
        received_signals, t = test_sim.simulate_target_echo(
            target_range, target_azimuth, target_elevation,
            noise_power=0.01 if param_name != 'noise_power' else param_val
        )
        
        detected_range, detected_az, detected_el, _ = test_sim.detect_target(
            received_signals, t
        )
        
        # Calculate error
        range_error = abs(detected_range - target_range)
        angle_error = np.sqrt((detected_az - target_azimuth)**2 + 
                             (detected_el - target_elevation)**2)
        total_error = range_error + angle_error * target_range  # Weighted error
        
        detection_errors.append(total_error)
    
    # Plot
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(param_values, detection_errors, 'o-')
    ax.set_xlabel(param_name.replace('_', ' ').title())
    ax.set_ylabel('Detection Error (m)')
    ax.set_title(f'Parameter Sweep: {param_name}')
    ax.grid(True)
    plt.tight_layout()
    plt.savefig(f'parameter_sweep_{param_name}.png', dpi=150)
    plt.show()
    
    return param_values, np.array(detection_errors)


def plot_array_geometry(mic_positions: np.ndarray, 
                        target_pos: Optional[np.ndarray] = None) -> None:
    """
    Visualize microphone array geometry and target position.
    
    Args:
        mic_positions: 4x3 array of microphone positions
        target_pos: Optional target position [x, y, z]
    """
    fig = plt.figure(figsize=(12, 5))
    
    # Top view (XY plane)
    ax1 = fig.add_subplot(121)
    ax1.scatter(mic_positions[:, 0] * 1000, mic_positions[:, 1] * 1000, 
               s=200, c='blue', marker='o', label='Microphones')
    for i, pos in enumerate(mic_positions):
        ax1.annotate(f'Mic {i}', (pos[0] * 1000, pos[1] * 1000), 
                    xytext=(5, 5), textcoords='offset points')
    if target_pos is not None:
        ax1.scatter(target_pos[0] * 1000, target_pos[1] * 1000,
                   s=300, c='red', marker='*', label='Target')
    ax1.set_xlabel('X (mm)')
    ax1.set_ylabel('Y (mm)')
    ax1.set_title('Array Geometry (Top View)')
    ax1.grid(True)
    ax1.legend()
    ax1.axis('equal')
    
    # Side view (XZ plane)
    ax2 = fig.add_subplot(122)
    ax2.scatter(mic_positions[:, 0] * 1000, mic_positions[:, 2] * 1000,
               s=200, c='blue', marker='o', label='Microphones')
    for i, pos in enumerate(mic_positions):
        ax2.annotate(f'Mic {i}', (pos[0] * 1000, pos[2] * 1000),
                    xytext=(5, 5), textcoords='offset points')
    if target_pos is not None:
        ax2.scatter(target_pos[0] * 1000, target_pos[2] * 1000,
                   s=300, c='red', marker='*', label='Target')
    ax2.set_xlabel('X (mm)')
    ax2.set_ylabel('Z (mm)')
    ax2.set_title('Array Geometry (Side View)')
    ax2.grid(True)
    ax2.legend()
    ax2.axis('equal')
    
    plt.tight_layout()
    plt.savefig('array_geometry.png', dpi=150)
    plt.show()


def analyze_detection_performance(simulator,
                                 num_trials: int = 100,
                                 target_range: float = 2.0,
                                 target_azimuth: float = 0.0,
                                 target_elevation: float = 0.0,
                                 noise_levels: List[float] = [0.001, 0.01, 0.1]) -> dict:
    """
    Analyze detection performance across multiple trials and noise levels.
    
    Args:
        simulator: SonarSimulator instance
        num_trials: Number of trials per noise level
        target_range: Target range
        target_azimuth: Target azimuth
        target_elevation: Target elevation
        noise_levels: List of noise power levels to test
        
    Returns:
        Dictionary with performance metrics
    """
    results = {
        'noise_levels': noise_levels,
        'detection_rate': [],
        'range_rmse': [],
        'angle_rmse': [],
        'snr_values': []
    }
    
    for noise_power in noise_levels:
        range_errors = []
        angle_errors = []
        detections = 0
        snr_list = []
        
        for trial in range(num_trials):
            # Simulate
            received_signals, t = simulator.simulate_target_echo(
                target_range, target_azimuth, target_elevation,
                noise_power=noise_power
            )
            
            # Calculate SNR
            peak_idx = int((2 * target_range / simulator.c) * simulator.fs)
            signal_window = (max(0, peak_idx - 50), min(len(received_signals[0]), peak_idx + 50))
            noise_window = (0, min(1000, len(received_signals[0]) // 10))
            snr = calculate_snr(received_signals[0, :], signal_window, noise_window)
            snr_list.append(snr)
            
            # Detect
            try:
                detected_range, detected_az, detected_el, beamformed = simulator.detect_target(
                    received_signals, t
                )
                
                range_error = abs(detected_range - target_range)
                angle_error = np.sqrt((detected_az - target_azimuth)**2 + 
                                     (detected_el - target_elevation)**2)
                
                range_errors.append(range_error)
                angle_errors.append(angle_error)
                detections += 1
            except:
                pass
        
        results['detection_rate'].append(detections / num_trials)
        results['range_rmse'].append(np.sqrt(np.mean(np.array(range_errors)**2)) if range_errors else np.inf)
        results['angle_rmse'].append(np.sqrt(np.mean(np.array(angle_errors)**2)) if angle_errors else np.inf)
        results['snr_values'].append(np.mean(snr_list))
    
    # Plot results
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    
    axes[0, 0].semilogx(noise_levels, results['detection_rate'], 'o-')
    axes[0, 0].set_xlabel('Noise Power')
    axes[0, 0].set_ylabel('Detection Rate')
    axes[0, 0].set_title('Detection Rate vs Noise')
    axes[0, 0].grid(True)
    
    axes[0, 1].semilogx(noise_levels, results['range_rmse'], 'o-')
    axes[0, 1].set_xlabel('Noise Power')
    axes[0, 1].set_ylabel('Range RMSE (m)')
    axes[0, 1].set_title('Range Accuracy vs Noise')
    axes[0, 1].grid(True)
    
    axes[1, 0].semilogx(noise_levels, results['angle_rmse'], 'o-')
    axes[1, 0].set_xlabel('Noise Power')
    axes[1, 0].set_ylabel('Angle RMSE (rad)')
    axes[1, 0].set_title('Angle Accuracy vs Noise')
    axes[1, 0].grid(True)
    
    axes[1, 1].semilogx(noise_levels, results['snr_values'], 'o-')
    axes[1, 1].set_xlabel('Noise Power')
    axes[1, 1].set_ylabel('SNR (dB)')
    axes[1, 1].set_title('SNR vs Noise')
    axes[1, 1].grid(True)
    
    plt.tight_layout()
    plt.savefig('detection_performance.png', dpi=150)
    plt.show()
    
    return results


if __name__ == '__main__':
    # Example usage
    from sonar_sim import SonarSimulator
    
    sim = SonarSimulator()
    
    print("Running analysis tools examples...")
    
    # Plot beam pattern
    print("1. Plotting beam pattern...")
    plot_beam_pattern(sim, target_azimuth=np.radians(15), target_elevation=np.radians(5))
    
    # Analyze performance
    print("2. Analyzing detection performance...")
    results = analyze_detection_performance(sim, num_trials=50)
    
    print("Analysis complete!")

