/**
 ******************************************************************************
 * @file    calibration.c
 * @brief   Calibration and diagnostic utilities implementation
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "calibration.h"
#include "arm_math.h"
#include <string.h>
#include <math.h>

/* Speed of sound vs temperature: c = 331.3 + 0.606 * T (m/s) */
#define SPEED_OF_SOUND_BASE       331.3f
#define SPEED_OF_SOUND_COEFF      0.606f

/**
 * @brief Initialize calibration system
 */
int32_t calibration_init(system_calibration_t *cal)
{
    if (cal == NULL) {
        return -1;
    }
    
    memset(cal, 0, sizeof(system_calibration_t));
    
    /* Initialize with default values */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        cal->mic_cal.gain[i] = 1.0f;
        cal->mic_cal.phase_offset[i] = 0.0f;
        cal->mic_cal.dc_offset[i] = 0.0f;
    }
    
    cal->mic_cal.calibrated = false;
    cal->speed_of_sound = SPEED_OF_SOUND_BASE + SPEED_OF_SOUND_COEFF * CALIBRATION_TEMPERATURE;
    cal->temperature = CALIBRATION_TEMPERATURE;
    cal->tx_power = 1.0f;
    cal->system_calibrated = false;
    
    return 0;
}

/**
 * @brief Calibrate microphone array
 */
int32_t calibration_calibrate_mics(system_calibration_t *cal,
                                    float32_t *reference_signal,
                                    uint32_t length)
{
    if (cal == NULL || reference_signal == NULL || length == 0) {
        return -1;
    }
    
    /* This is a placeholder - actual calibration would:
     * 1. Transmit known reference signal
     * 2. Measure response from each microphone
     * 3. Calculate gain, phase, and DC offset corrections
     * 4. Store in calibration structure
     */
    
    /* For now, assume unity gain and zero offsets */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        cal->mic_cal.gain[i] = 1.0f;
        cal->mic_cal.phase_offset[i] = 0.0f;
        cal->mic_cal.dc_offset[i] = 0.0f;
    }
    
    cal->mic_cal.calibrated = true;
    
    return 0;
}

/**
 * @brief Calibrate speed of sound based on temperature
 */
int32_t calibration_set_temperature(system_calibration_t *cal, float temperature)
{
    if (cal == NULL) {
        return -1;
    }
    
    cal->temperature = temperature;
    cal->speed_of_sound = SPEED_OF_SOUND_BASE + SPEED_OF_SOUND_COEFF * temperature;
    
    return 0;
}

/**
 * @brief Apply calibration corrections to received signals
 */
void calibration_apply(system_calibration_t *cal,
                      float32_t *raw_signals[NUM_MICS],
                      float32_t *calibrated_signals[NUM_MICS],
                      uint32_t length)
{
    if (cal == NULL || raw_signals == NULL || calibrated_signals == NULL) {
        return;
    }
    
    if (!cal->mic_cal.calibrated) {
        /* No calibration - just copy */
        for (uint32_t i = 0; i < NUM_MICS; i++) {
            memcpy(calibrated_signals[i], raw_signals[i], length * sizeof(float32_t));
        }
        return;
    }
    
    /* Apply gain, DC offset, and phase correction */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        float32_t gain = cal->mic_cal.gain[i];
        float32_t dc_offset = cal->mic_cal.dc_offset[i];
        
        /* Apply gain and DC offset correction */
        for (uint32_t j = 0; j < length; j++) {
            calibrated_signals[i][j] = (raw_signals[i][j] - dc_offset) * gain;
        }
        
        /* Phase correction would require FFT/IFFT - simplified here */
        /* In practice, phase correction is applied during beamforming */
    }
}

/**
 * @brief Run system diagnostics
 */
int32_t calibration_run_diagnostics(system_calibration_t *cal,
                                    float32_t *rx_signals[NUM_MICS],
                                    uint32_t length,
                                    diagnostic_data_t *diag)
{
    if (cal == NULL || rx_signals == NULL || diag == NULL || length == 0) {
        return -1;
    }
    
    /* Allocate diagnostic buffers if needed */
    if (diag->signal_power == NULL) {
        diag->signal_power = (float32_t *)malloc(NUM_MICS * sizeof(float32_t));
        diag->noise_floor = (float32_t *)malloc(NUM_MICS * sizeof(float32_t));
    }
    
    if (diag->signal_power == NULL || diag->noise_floor == NULL) {
        return -2;
    }
    
    /* Analyze each channel */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        /* Calculate signal power (RMS) */
        float32_t rms;
        arm_rms_f32(rx_signals[i], length, &rms);
        diag->signal_power[i] = rms * rms;
        
        /* Estimate noise floor (use first 10% of samples) */
        uint32_t noise_samples = length / 10;
        if (noise_samples > 0) {
            float32_t noise_rms;
            arm_rms_f32(rx_signals[i], noise_samples, &noise_rms);
            diag->noise_floor[i] = noise_rms * noise_rms;
        } else {
            diag->noise_floor[i] = diag->signal_power[i];
        }
        
        /* Calculate SNR in dB */
        if (diag->noise_floor[i] > 0.0f) {
            float32_t snr_linear = diag->signal_power[i] / diag->noise_floor[i];
            diag->snr_db[i] = 10.0f * log10f(snr_linear);
        } else {
            diag->snr_db[i] = 0.0f;
        }
    }
    
    diag->sample_count = length;
    diag->valid = true;
    
    return 0;
}

/**
 * @brief Get calibration status
 */
bool calibration_is_calibrated(system_calibration_t *cal)
{
    if (cal == NULL) {
        return false;
    }
    
    return cal->system_calibrated && cal->mic_cal.calibrated;
}

/**
 * @brief Save calibration to non-volatile memory
 */
int32_t calibration_save(system_calibration_t *cal)
{
    if (cal == NULL) {
        return -1;
    }
    
    /* Placeholder - would write to flash/EEPROM */
    /* In practice, use HAL_FLASH_Program or similar */
    
    return 0;
}

/**
 * @brief Load calibration from non-volatile memory
 */
int32_t calibration_load(system_calibration_t *cal)
{
    if (cal == NULL) {
        return -1;
    }
    
    /* Placeholder - would read from flash/EEPROM */
    /* In practice, use HAL_FLASH_Read or similar */
    
    return 0;
}

