/**
 ******************************************************************************
 * @file    sonar.h
 * @brief   Active sonar processing header for SkeeterHawk
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef SONAR_H
#define SONAR_H

#include <stdint.h>
#include <stdbool.h>
#include "arm_math.h"

/* Sonar Configuration */
#define SONAR_SAMPLE_RATE         200000U      // 200 kHz
#define SONAR_CHIRP_DURATION_MS   1U           // 1 ms chirp
#define SONAR_CHIRP_F0            38000U       // 38 kHz start
#define SONAR_CHIRP_F1            42000U       // 42 kHz end
#define SONAR_CHIRP_SAMPLES        (SONAR_SAMPLE_RATE * SONAR_CHIRP_DURATION_MS / 1000)
#define SONAR_MAX_RANGE_CM        500U         // 5 meters max range
#define SONAR_MAX_SAMPLES          (SONAR_SAMPLE_RATE * SONAR_MAX_RANGE_CM * 2 / 34300)

/* Microphone Array Configuration */
#define NUM_MICS                  4U
#define MIC_ARRAY_SPACING_MM      10U           // 1 cm spacing

/* Beamforming Configuration */
#define BEAMFORM_AZIMUTH_STEPS    20U
#define BEAMFORM_ELEVATION_STEPS  20U
#define BEAMFORM_AZIMUTH_MIN      (-M_PI / 2)
#define BEAMFORM_AZIMUTH_MAX      (M_PI / 2)
#define BEAMFORM_ELEVATION_MIN    (-M_PI / 4)
#define BEAMFORM_ELEVATION_MAX    (M_PI / 4)

/* Detection Thresholds */
#define DETECTION_THRESHOLD       1000.0f      // Minimum peak amplitude
#define MIN_RANGE_CM             10U           // 10 cm minimum
#define MAX_RANGE_CM             500U          // 5 meters maximum

/* Target Information Structure */
typedef struct {
    float range_cm;              // Range in cm
    float azimuth_rad;           // Azimuth in radians
    float elevation_rad;         // Elevation in radians
    float confidence;            // Detection confidence (0-1)
    bool valid;                  // Valid detection flag
} target_info_t;

/* Sonar State Structure */
typedef struct {
    float32_t *tx_chirp;         // Transmit chirp signal
    float32_t *matched_filter;   // Matched filter coefficients
    float32_t *rx_buffer[NUM_MICS]; // Receive buffers per mic
    float32_t *filtered_buffer[NUM_MICS]; // Matched filter output
    float32_t *beamformed_output; // Beamformed output
    target_info_t target;        // Detected target
    bool processing_complete;    // Processing status
    uint32_t sample_count;       // Current sample count
} sonar_state_t;

/* Function Prototypes */

/**
 * @brief Initialize sonar system
 * @param state Pointer to sonar state structure
 * @return 0 on success, negative on error
 */
int32_t sonar_init(sonar_state_t *state);

/**
 * @brief Generate transmit chirp signal
 * @param chirp Output buffer for chirp
 * @param length Length of chirp buffer
 */
void sonar_generate_chirp(float32_t *chirp, uint32_t length);

/**
 * @brief Process received signals with matched filter
 * @param state Pointer to sonar state
 * @param mic_idx Microphone index (0-3)
 */
void sonar_matched_filter(sonar_state_t *state, uint32_t mic_idx);

/**
 * @brief Perform delay-and-sum beamforming
 * @param state Pointer to sonar state
 * @param azimuth Steering azimuth in radians
 * @param elevation Steering elevation in radians
 */
void sonar_beamform(sonar_state_t *state, float azimuth, float elevation);

/**
 * @brief Detect target using matched filtering and beamforming
 * @param state Pointer to sonar state
 * @return 0 on success, negative on error
 */
int32_t sonar_detect_target(sonar_state_t *state);

/**
 * @brief Get detected target information
 * @param state Pointer to sonar state
 * @param target Output target information structure
 * @return 0 on success, negative if no target detected
 */
int32_t sonar_get_target(sonar_state_t *state, target_info_t *target);

/**
 * @brief Deinitialize sonar system
 * @param state Pointer to sonar state structure
 */
void sonar_deinit(sonar_state_t *state);

#endif /* SONAR_H */

