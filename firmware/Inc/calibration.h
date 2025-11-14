/**
 ******************************************************************************
 * @file    calibration.h
 * @brief   Calibration and diagnostic utilities for SkeeterHawk
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>
#include "sonar.h"

/* Calibration Configuration */
#define CALIBRATION_SAMPLES       1000U
#define CALIBRATION_TEMPERATURE   20.0f  // Reference temperature (°C)

/* Microphone Calibration Structure */
typedef struct {
    float gain[NUM_MICS];           // Gain correction per mic
    float phase_offset[NUM_MICS];   // Phase offset per mic
    float dc_offset[NUM_MICS];      // DC offset per mic
    bool calibrated;                // Calibration status
} mic_calibration_t;

/* System Calibration Structure */
typedef struct {
    mic_calibration_t mic_cal;
    float speed_of_sound;           // Calibrated speed of sound (m/s)
    float temperature;               // Current temperature (°C)
    float tx_power;                 // Transmit power calibration
    bool system_calibrated;         // Overall calibration status
} system_calibration_t;

/* Diagnostic Data Structure */
typedef struct {
    float32_t *signal_power;        // Signal power per channel
    float32_t *noise_floor;         // Noise floor per channel
    float32_t snr_db[NUM_MICS];     // SNR per channel
    uint32_t sample_count;          // Number of samples analyzed
    bool valid;                     // Valid diagnostic data
} diagnostic_data_t;

/* Function Prototypes */

/**
 * @brief Initialize calibration system
 * @param cal Pointer to calibration structure
 * @return 0 on success, negative on error
 */
int32_t calibration_init(system_calibration_t *cal);

/**
 * @brief Calibrate microphone array
 * @param cal Pointer to calibration structure
 * @param reference_signal Reference signal for calibration
 * @param length Reference signal length
 * @return 0 on success, negative on error
 */
int32_t calibration_calibrate_mics(system_calibration_t *cal,
                                    float32_t *reference_signal,
                                    uint32_t length);

/**
 * @brief Calibrate speed of sound based on temperature
 * @param cal Pointer to calibration structure
 * @param temperature Temperature in Celsius
 * @return 0 on success, negative on error
 */
int32_t calibration_set_temperature(system_calibration_t *cal, float temperature);

/**
 * @brief Apply calibration corrections to received signals
 * @param cal Pointer to calibration structure
 * @param raw_signals Raw input signals (4 channels)
 * @param calibrated_signals Output calibrated signals
 * @param length Number of samples
 */
void calibration_apply(system_calibration_t *cal,
                        float32_t *raw_signals[NUM_MICS],
                        float32_t *calibrated_signals[NUM_MICS],
                        uint32_t length);

/**
 * @brief Run system diagnostics
 * @param cal Pointer to calibration structure
 * @param rx_signals Received signals for analysis
 * @param length Number of samples
 * @param diag Output diagnostic data
 * @return 0 on success, negative on error
 */
int32_t calibration_run_diagnostics(system_calibration_t *cal,
                                    float32_t *rx_signals[NUM_MICS],
                                    uint32_t length,
                                    diagnostic_data_t *diag);

/**
 * @brief Get calibration status
 * @param cal Pointer to calibration structure
 * @return true if calibrated, false otherwise
 */
bool calibration_is_calibrated(system_calibration_t *cal);

/**
 * @brief Save calibration to non-volatile memory
 * @param cal Pointer to calibration structure
 * @return 0 on success, negative on error
 */
int32_t calibration_save(system_calibration_t *cal);

/**
 * @brief Load calibration from non-volatile memory
 * @param cal Pointer to calibration structure
 * @return 0 on success, negative on error
 */
int32_t calibration_load(system_calibration_t *cal);

#endif /* CALIBRATION_H */

