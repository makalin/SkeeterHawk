/**
 ******************************************************************************
 * @file    signal_utils.h
 * @brief   Signal processing utility functions
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef SIGNAL_UTILS_H
#define SIGNAL_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "arm_math.h"

/* Adaptive Threshold Configuration */
#define ADAPTIVE_THRESHOLD_WINDOW  100U
#define ADAPTIVE_THRESHOLD_FACTOR  3.0f  // N-sigma threshold

/* Multi-Target Configuration */
#define MAX_TARGETS                5U
#define MIN_TARGET_SEPARATION_CM  20U    // Minimum separation between targets

/* Target Cluster */
typedef struct {
    float range_cm;
    float azimuth_rad;
    float elevation_rad;
    float power;
    uint32_t sample_count;
} target_cluster_t;

/* Multi-Target Detection Result */
typedef struct {
    target_cluster_t targets[MAX_TARGETS];
    uint32_t num_targets;
    bool valid;
} multi_target_result_t;

/* Function Prototypes */

/**
 * @brief Apply adaptive threshold to signal
 * @param signal Input signal
 * @param length Signal length
 * @param threshold Output threshold value
 * @return 0 on success, negative on error
 */
int32_t signal_adaptive_threshold(float32_t *signal,
                                  uint32_t length,
                                  float32_t *threshold);

/**
 * @brief Find peaks in signal above threshold
 * @param signal Input signal
 * @param length Signal length
 * @param threshold Detection threshold
 * @param peaks Output peak indices
 * @param max_peaks Maximum number of peaks to find
 * @param num_peaks Output number of peaks found
 * @return 0 on success, negative on error
 */
int32_t signal_find_peaks(float32_t *signal,
                         uint32_t length,
                         float32_t threshold,
                         uint32_t *peaks,
                         uint32_t max_peaks,
                         uint32_t *num_peaks);

/**
 * @brief Cluster nearby detections
 * @param peaks Peak indices
 * @param num_peaks Number of peaks
 * @param signal Signal data for power estimation
 * @param length Signal length
 * @param sample_rate Sample rate in Hz
 * @param clusters Output clusters
 * @param max_clusters Maximum number of clusters
 * @param num_clusters Output number of clusters
 * @return 0 on success, negative on error
 */
int32_t signal_cluster_detections(uint32_t *peaks,
                                  uint32_t num_peaks,
                                  float32_t *signal,
                                  uint32_t length,
                                  uint32_t sample_rate,
                                  target_cluster_t *clusters,
                                  uint32_t max_clusters,
                                  uint32_t *num_clusters);

/**
 * @brief Detect multiple targets
 * @param beamformed_output Beamformed signal
 * @param length Signal length
 * @param sample_rate Sample rate in Hz
 * @param result Output multi-target result
 * @return 0 on success, negative on error
 */
int32_t signal_detect_multi_target(float32_t *beamformed_output,
                                   uint32_t length,
                                   uint32_t sample_rate,
                                   multi_target_result_t *result);

/**
 * @brief Apply bandpass filter
 * @param input Input signal
 * @param output Output signal
 * @param length Signal length
 * @param low_freq Low cutoff frequency (Hz)
 * @param high_freq High cutoff frequency (Hz)
 * @param sample_rate Sample rate (Hz)
 * @return 0 on success, negative on error
 */
int32_t signal_bandpass_filter(float32_t *input,
                               float32_t *output,
                               uint32_t length,
                               float32_t low_freq,
                               float32_t high_freq,
                               uint32_t sample_rate);

/**
 * @brief Calculate signal statistics
 * @param signal Input signal
 * @param length Signal length
 * @param mean Output mean value
 * @param std Output standard deviation
 * @param peak Output peak value
 * @return 0 on success, negative on error
 */
int32_t signal_calculate_stats(float32_t *signal,
                              uint32_t length,
                              float32_t *mean,
                              float32_t *std,
                              float32_t *peak);

/**
 * @brief Normalize signal to [-1, 1] range
 * @param signal Input/output signal
 * @param length Signal length
 */
void signal_normalize(float32_t *signal, uint32_t length);

/**
 * @brief Apply window function (Hanning)
 * @param signal Input/output signal
 * @param length Signal length
 */
void signal_apply_window(float32_t *signal, uint32_t length);

#endif /* SIGNAL_UTILS_H */

