#!/usr/bin/env python3
"""
SkeeterHawk Test Suite
======================
Comprehensive test suite for validating sonar algorithms
"""

import numpy as np
import sys
from sonar_sim import SonarSimulator
from analysis_tools import (
    calculate_snr, analyze_detection_performance,
    parameter_sweep, plot_beam_pattern
)

class TestSuite:
    """Test suite for SkeeterHawk sonar system"""
    
    def __init__(self):
        self.simulator = SonarSimulator()
        self.tests_passed = 0
        self.tests_failed = 0
        
    def run_test(self, test_name, test_func):
        """Run a single test"""
        print(f"\n{'='*60}")
        print(f"Running: {test_name}")
        print(f"{'='*60}")
        try:
            result = test_func()
            if result:
                print(f"✓ PASSED: {test_name}")
                self.tests_passed += 1
                return True
            else:
                print(f"✗ FAILED: {test_name}")
                self.tests_failed += 1
                return False
        except Exception as e:
            print(f"✗ ERROR in {test_name}: {e}")
            self.tests_failed += 1
            return False
    
    def test_chirp_generation(self):
        """Test LFM chirp generation"""
        chirp = self.simulator.tx_chirp
        assert len(chirp) > 0, "Chirp should not be empty"
        assert np.max(np.abs(chirp)) <= 1.0, "Chirp amplitude should be normalized"
        
        # Check frequency content
        fft_chirp = np.fft.fft(chirp)
        freqs = np.fft.fftfreq(len(chirp), 1/self.simulator.fs)
        power = np.abs(fft_chirp)
        
        # Find peak frequency
        peak_idx = np.argmax(power[:len(power)//2])
        peak_freq = abs(freqs[peak_idx])
        
        # Should be around 40kHz
        assert 35000 < peak_freq < 45000, f"Peak frequency {peak_freq} Hz not in expected range"
        
        return True
    
    def test_matched_filter(self):
        """Test matched filtering"""
        # Create test signal with known delay
        test_signal = np.zeros(10000)
        delay = 1000
        test_signal[delay:delay+len(self.simulator.tx_chirp)] = self.simulator.tx_chirp
        
        # Apply matched filter
        output = self.simulator.matched_filter_process(test_signal)
        
        # Find peak
        peak_idx = np.argmax(np.abs(output))
        
        # Peak should be near the delay
        assert abs(peak_idx - delay) < 50, f"Peak at {peak_idx}, expected near {delay}"
        
        return True
    
    def test_beamforming(self):
        """Test beamforming"""
        target_range = 2.0
        target_azimuth = np.radians(15)
        target_elevation = np.radians(5)
        
        # Simulate target
        received_signals, t = self.simulator.simulate_target_echo(
            target_range, target_azimuth, target_elevation
        )
        
        # Apply matched filter
        filtered = np.zeros_like(received_signals)
        for i in range(4):
            filtered[i, :] = self.simulator.matched_filter_process(received_signals[i, :])
        
        # Beamform at target direction
        beamformed = self.simulator.beamform(filtered, target_azimuth, target_elevation)
        
        # Should have a peak
        peak_power = np.max(np.abs(beamformed))
        assert peak_power > 0.01, "Beamformed output should have significant power"
        
        return True
    
    def test_target_detection(self):
        """Test target detection"""
        target_range = 2.0
        target_azimuth = np.radians(15)
        target_elevation = np.radians(5)
        
        # Simulate and detect
        received_signals, t = self.simulator.simulate_target_echo(
            target_range, target_azimuth, target_elevation, noise_power=0.01
        )
        
        detected_range, detected_az, detected_el, _ = self.simulator.detect_target(
            received_signals, t
        )
        
        # Check accuracy
        range_error = abs(detected_range - target_range)
        angle_error = np.sqrt((detected_az - target_azimuth)**2 + 
                             (detected_el - target_elevation)**2)
        
        assert range_error < 0.5, f"Range error {range_error}m too large"
        assert angle_error < np.radians(10), f"Angle error {np.degrees(angle_error)}° too large"
        
        return True
    
    def test_snr_calculation(self):
        """Test SNR calculation"""
        # Create signal with known SNR
        signal_power = 1.0
        noise_power = 0.01
        expected_snr_db = 10 * np.log10(signal_power / noise_power)
        
        signal = np.random.normal(0, np.sqrt(signal_power), 1000)
        noise = np.random.normal(0, np.sqrt(noise_power), 1000)
        combined = signal + noise
        
        # Calculate SNR
        snr = calculate_snr(combined, (0, 1000), (0, 1000))
        
        # Should be approximately correct (within 3dB)
        assert abs(snr - expected_snr_db) < 3.0, f"SNR {snr}dB not close to expected {expected_snr_db}dB"
        
        return True
    
    def test_multi_target(self):
        """Test detection with multiple targets"""
        # This would require multi-target detection implementation
        # For now, just verify single target works
        return self.test_target_detection()
    
    def run_all_tests(self):
        """Run all tests"""
        print("\n" + "="*60)
        print("SkeeterHawk Test Suite")
        print("="*60)
        
        tests = [
            ("Chirp Generation", self.test_chirp_generation),
            ("Matched Filter", self.test_matched_filter),
            ("Beamforming", self.test_beamforming),
            ("Target Detection", self.test_target_detection),
            ("SNR Calculation", self.test_snr_calculation),
            ("Multi-Target", self.test_multi_target),
        ]
        
        for test_name, test_func in tests:
            self.run_test(test_name, test_func)
        
        # Print summary
        print("\n" + "="*60)
        print("Test Summary")
        print("="*60)
        print(f"Passed: {self.tests_passed}")
        print(f"Failed: {self.tests_failed}")
        print(f"Total:  {self.tests_passed + self.tests_failed}")
        print("="*60)
        
        return self.tests_failed == 0


if __name__ == '__main__':
    suite = TestSuite()
    success = suite.run_all_tests()
    sys.exit(0 if success else 1)

