#!/usr/bin/env python3
"""
SkeeterHawk Active Sonar Simulation
====================================
Simulates the matched filter and beamforming pipeline for mosquito detection
using active ultrasonic echolocation.

Author: Mehmet T. AKALIN
License: MIT
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft, ifft, fftfreq
from typing import Tuple, List
import warnings
warnings.filterwarnings('ignore')


class SonarSimulator:
    """
    Simulates active sonar system with matched filtering and beamforming.
    """
    
    def __init__(self, 
                 sample_rate: float = 200000,  # 200 kHz
                 chirp_duration: float = 0.001,  # 1 ms
                 chirp_f0: float = 38000,  # 38 kHz
                 chirp_f1: float = 42000,  # 42 kHz
                 speed_of_sound: float = 343.0,  # m/s at 20°C
                 mic_array_positions: np.ndarray = None):
        """
        Initialize sonar simulator.
        
        Args:
            sample_rate: Sampling rate in Hz
            chirp_duration: Duration of LFM chirp in seconds
            chirp_f0: Start frequency of chirp in Hz
            chirp_f1: End frequency of chirp in Hz
            speed_of_sound: Speed of sound in m/s
            mic_array_positions: 4x3 array of microphone positions in meters
        """
        self.fs = sample_rate
        self.chirp_duration = chirp_duration
        self.chirp_f0 = chirp_f0
        self.chirp_f1 = chirp_f1
        self.c = speed_of_sound
        
        # Default 2x2 microphone array positions (in meters)
        if mic_array_positions is None:
            array_spacing = 0.01  # 1 cm spacing
            self.mic_positions = np.array([
                [-array_spacing/2, -array_spacing/2, 0],
                [array_spacing/2, -array_spacing/2, 0],
                [-array_spacing/2, array_spacing/2, 0],
                [array_spacing/2, array_spacing/2, 0]
            ])
        else:
            self.mic_positions = mic_array_positions
        
        # Generate transmit chirp
        self.tx_chirp = self._generate_chirp()
        
        # Matched filter (time-reversed chirp)
        self.matched_filter = np.flip(self.tx_chirp)
        
    def _generate_chirp(self) -> np.ndarray:
        """
        Generate Linear Frequency Modulated (LFM) chirp.
        
        Returns:
            Transmit chirp signal
        """
        t = np.linspace(0, self.chirp_duration, int(self.fs * self.chirp_duration))
        bandwidth = self.chirp_f1 - self.chirp_f0
        chirp_rate = bandwidth / self.chirp_duration
        
        # LFM chirp: s(t) = A * cos(2π * (f0*t + 0.5*chirp_rate*t²))
        phase = 2 * np.pi * (self.chirp_f0 * t + 0.5 * chirp_rate * t**2)
        chirp = np.cos(phase)
        
        # Apply window to reduce sidelobes
        window = np.hanning(len(chirp))
        chirp = chirp * window
        
        return chirp
    
    def simulate_target_echo(self,
                            target_range: float,
                            target_azimuth: float,
                            target_elevation: float,
                            target_rcs: float = 1e-6,
                            noise_power: float = 0.01) -> Tuple[np.ndarray, np.ndarray]:
        """
        Simulate received echo from a target at given position.
        
        Args:
            target_range: Range to target in meters
            target_azimuth: Azimuth angle in radians (0 = forward)
            target_elevation: Elevation angle in radians (0 = horizontal)
            target_rcs: Radar Cross Section (target reflectivity)
            noise_power: Noise power level
            
        Returns:
            Tuple of (received signals per mic, time vector)
        """
        # Time of flight
        tof = 2 * target_range / self.c
        
        # Time vector for received signal
        max_range = 5.0  # Maximum detectable range
        t_max = 2 * max_range / self.c
        t = np.arange(0, t_max, 1/self.fs)
        
        # Received signal amplitude (inverse square law + RCS)
        amplitude = np.sqrt(target_rcs) / (target_range ** 2)
        
        # Create echo signal (delayed and attenuated chirp)
        echo = np.zeros_like(t)
        echo_start_idx = int(tof * self.fs)
        echo_end_idx = echo_start_idx + len(self.tx_chirp)
        
        if echo_end_idx < len(echo):
            echo[echo_start_idx:echo_end_idx] = amplitude * self.tx_chirp
        
        # Calculate TDOA for each microphone
        target_pos = np.array([
            target_range * np.cos(target_elevation) * np.cos(target_azimuth),
            target_range * np.cos(target_elevation) * np.sin(target_azimuth),
            target_range * np.sin(target_elevation)
        ])
        
        # Received signals at each microphone
        received_signals = np.zeros((4, len(t)))
        
        for i, mic_pos in enumerate(self.mic_positions):
            # Distance from target to microphone
            dist_to_mic = np.linalg.norm(target_pos - mic_pos)
            tof_to_mic = dist_to_mic / self.c
            
            # Time delay relative to first mic
            tdoa = tof_to_mic - (np.linalg.norm(target_pos - self.mic_positions[0]) / self.c)
            
            # Shift echo by TDOA
            delay_samples = int(tdoa * self.fs)
            if delay_samples != 0:
                if delay_samples > 0:
                    received_signals[i, delay_samples:] = echo[:-delay_samples] if delay_samples < len(echo) else 0
                else:
                    received_signals[i, :delay_samples] = echo[-delay_samples:] if abs(delay_samples) < len(echo) else 0
            else:
                received_signals[i, :] = echo
            
            # Add noise
            noise = np.random.normal(0, np.sqrt(noise_power), len(t))
            received_signals[i, :] += noise
        
        return received_signals, t
    
    def matched_filter_process(self, received_signal: np.ndarray) -> np.ndarray:
        """
        Apply matched filter (pulse compression) to received signal.
        
        Args:
            received_signal: Received signal from one microphone
            
        Returns:
            Compressed output
        """
        # Cross-correlation (matched filtering)
        output = np.correlate(received_signal, self.matched_filter, mode='same')
        return output
    
    def beamform(self,
                 received_signals: np.ndarray,
                 azimuth: float,
                 elevation: float) -> np.ndarray:
        """
        Delay-and-sum beamforming for given steering direction.
        
        Args:
            received_signals: 4xN array of received signals
            azimuth: Steering azimuth in radians
            elevation: Steering elevation in radians
            
        Returns:
            Beamformed output signal
        """
        # Steering direction vector
        steering_dir = np.array([
            np.cos(elevation) * np.cos(azimuth),
            np.cos(elevation) * np.sin(azimuth),
            np.sin(elevation)
        ])
        
        # Calculate delays for each microphone
        delays = np.zeros(4)
        for i, mic_pos in enumerate(self.mic_positions):
            # Project mic position onto steering direction
            delays[i] = np.dot(mic_pos, steering_dir) / self.c
        
        # Normalize delays relative to first mic
        delays = delays - delays[0]
        
        # Delay and sum
        beamformed = np.zeros(received_signals.shape[1])
        for i in range(4):
            delay_samples = int(delays[i] * self.fs)
            if delay_samples != 0:
                if delay_samples > 0:
                    beamformed[delay_samples:] += received_signals[i, :-delay_samples] if delay_samples < len(beamformed) else 0
                else:
                    beamformed[:delay_samples] += received_signals[i, -delay_samples:] if abs(delay_samples) < len(beamformed) else 0
            else:
                beamformed += received_signals[i, :]
        
        return beamformed / 4.0  # Average
    
    def detect_target(self,
                     received_signals: np.ndarray,
                     t: np.ndarray,
                     azimuth_range: Tuple[float, float] = (-np.pi/2, np.pi/2),
                     elevation_range: Tuple[float, float] = (-np.pi/4, np.pi/4),
                     num_angles: int = 20) -> Tuple[float, float, float, np.ndarray]:
        """
        Detect target using matched filtering and beamforming.
        
        Args:
            received_signals: 4xN array of received signals
            t: Time vector
            azimuth_range: Range of azimuth angles to search
            elevation_range: Range of elevation angles to search
            num_angles: Number of angles to search in each dimension
            
        Returns:
            Tuple of (detected_range, detected_azimuth, detected_elevation, beamformed_output)
        """
        # First apply matched filter to each channel
        filtered_signals = np.zeros_like(received_signals)
        for i in range(4):
            filtered_signals[i, :] = self.matched_filter_process(received_signals[i, :])
        
        # Search over angles
        azimuths = np.linspace(azimuth_range[0], azimuth_range[1], num_angles)
        elevations = np.linspace(elevation_range[0], elevation_range[1], num_angles)
        
        max_power = -np.inf
        best_azimuth = 0.0
        best_elevation = 0.0
        best_beamformed = None
        
        for az in azimuths:
            for el in elevations:
                beamformed = self.beamform(filtered_signals, az, el)
                power = np.max(np.abs(beamformed))
                
                if power > max_power:
                    max_power = power
                    best_azimuth = az
                    best_elevation = el
                    best_beamformed = beamformed
        
        # Estimate range from peak location
        peak_idx = np.argmax(np.abs(best_beamformed))
        detected_range = (t[peak_idx] * self.c) / 2.0
        
        return detected_range, best_azimuth, best_elevation, best_beamformed


def main():
    """
    Main simulation function.
    """
    print("SkeeterHawk Active Sonar Simulation")
    print("=" * 50)
    
    # Initialize simulator
    sim = SonarSimulator()
    
    # Simulate target
    target_range = 2.0  # 2 meters
    target_azimuth = np.radians(15)  # 15 degrees
    target_elevation = np.radians(5)  # 5 degrees
    
    print(f"\nSimulating target at:")
    print(f"  Range: {target_range:.2f} m")
    print(f"  Azimuth: {np.degrees(target_azimuth):.1f}°")
    print(f"  Elevation: {np.degrees(target_elevation):.1f}°")
    
    # Generate received signals
    received_signals, t = sim.simulate_target_echo(
        target_range, target_azimuth, target_elevation,
        target_rcs=1e-6, noise_power=0.01
    )
    
    # Detect target
    print("\nProcessing signals...")
    detected_range, detected_azimuth, detected_elevation, beamformed = sim.detect_target(
        received_signals, t
    )
    
    print(f"\nDetection Results:")
    print(f"  Detected Range: {detected_range:.2f} m (actual: {target_range:.2f} m)")
    print(f"  Detected Azimuth: {np.degrees(detected_azimuth):.1f}° (actual: {np.degrees(target_azimuth):.1f}°)")
    print(f"  Detected Elevation: {np.degrees(detected_elevation):.1f}° (actual: {np.degrees(target_elevation):.1f}°)")
    
    # Plot results
    fig, axes = plt.subplots(3, 1, figsize=(12, 10))
    
    # Plot received signals
    axes[0].set_title('Received Signals (4 Microphones)')
    for i in range(4):
        axes[0].plot(t * 1000, received_signals[i, :], label=f'Mic {i+1}', alpha=0.7)
    axes[0].set_xlabel('Time (ms)')
    axes[0].set_ylabel('Amplitude')
    axes[0].legend()
    axes[0].grid(True)
    
    # Plot matched filter output
    axes[1].set_title('Matched Filter Output (Pulse Compression)')
    matched_output = sim.matched_filter_process(received_signals[0, :])
    axes[1].plot(t * 1000, np.abs(matched_output))
    axes[1].set_xlabel('Time (ms)')
    axes[1].set_ylabel('Amplitude')
    axes[1].grid(True)
    
    # Plot beamformed output
    axes[2].set_title('Beamformed Output (Target Detection)')
    axes[2].plot(t * 1000, np.abs(beamformed))
    axes[2].axvline(detected_range * 2 / sim.c * 1000, color='r', linestyle='--', 
                    label=f'Detected Range: {detected_range:.2f}m')
    axes[2].set_xlabel('Time (ms)')
    axes[2].set_ylabel('Amplitude')
    axes[2].legend()
    axes[2].grid(True)
    
    plt.tight_layout()
    plt.savefig('sonar_simulation_results.png', dpi=150)
    print("\nPlot saved to 'sonar_simulation_results.png'")
    plt.show()


if __name__ == '__main__':
    main()

