/**
 ******************************************************************************
 * @file    signal_utils.c
 * @brief   Signal processing utility functions implementation
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "signal_utils.h"
#include "sonar.h"
#include "arm_math.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SPEED_OF_SOUND 343.0f

/**
 * @brief Apply adaptive threshold
 */
int32_t signal_adaptive_threshold(float32_t *signal,
                                  uint32_t length,
                                  float32_t *threshold)
{
    if (signal == NULL || threshold == NULL || length == 0) {
        return -1;
    }
    
    /* Calculate mean and standard deviation */
    float32_t mean, std;
    arm_mean_f32(signal, length, &mean);
    arm_std_f32(signal, length, &std);
    
    /* Adaptive threshold: mean + N * std */
    *threshold = mean + ADAPTIVE_THRESHOLD_FACTOR * std;
    
    return 0;
}

/**
 * @brief Find peaks in signal
 */
int32_t signal_find_peaks(float32_t *signal,
                          uint32_t length,
                          float32_t threshold,
                          uint32_t *peaks,
                          uint32_t max_peaks,
                          uint32_t *num_peaks)
{
    if (signal == NULL || peaks == NULL || num_peaks == NULL || length == 0) {
        return -1;
    }
    
    *num_peaks = 0;
    
    /* Simple peak detection: find local maxima above threshold */
    for (uint32_t i = 1; i < length - 1 && *num_peaks < max_peaks; i++) {
        float32_t val = fabsf(signal[i]);
        
        /* Check if peak (local maximum) */
        if (val > threshold &&
            val > fabsf(signal[i-1]) &&
            val > fabsf(signal[i+1])) {
            peaks[*num_peaks] = i;
            (*num_peaks)++;
        }
    }
    
    return 0;
}

/**
 * @brief Cluster nearby detections
 */
int32_t signal_cluster_detections(uint32_t *peaks,
                                  uint32_t num_peaks,
                                  float32_t *signal,
                                  uint32_t length,
                                  uint32_t sample_rate,
                                  target_cluster_t *clusters,
                                  uint32_t max_clusters,
                                  uint32_t *num_clusters)
{
    if (peaks == NULL || signal == NULL || clusters == NULL || 
        num_clusters == NULL || num_peaks == 0) {
        return -1;
    }
    
    *num_clusters = 0;
    
    /* Simple clustering: group peaks within minimum separation */
    bool *processed = (bool *)calloc(num_peaks, sizeof(bool));
    if (processed == NULL) {
        return -2;
    }
    
    for (uint32_t i = 0; i < num_peaks && *num_clusters < max_clusters; i++) {
        if (processed[i]) {
            continue;
        }
        
        /* Start new cluster */
        target_cluster_t *cluster = &clusters[*num_clusters];
        cluster->range_cm = (peaks[i] * SPEED_OF_SOUND * 100.0f) / (2.0f * sample_rate);
        cluster->power = fabsf(signal[peaks[i]]);
        cluster->sample_count = 1;
        processed[i] = true;
        
        /* Find nearby peaks */
        for (uint32_t j = i + 1; j < num_peaks; j++) {
            if (processed[j]) {
                continue;
            }
            
            float range_j = (peaks[j] * SPEED_OF_SOUND * 100.0f) / (2.0f * sample_rate);
            float separation = fabsf(range_j - cluster->range_cm);
            
            if (separation < MIN_TARGET_SEPARATION_CM) {
                /* Merge into cluster */
                cluster->range_cm = (cluster->range_cm * cluster->sample_count + range_j) / 
                                   (cluster->sample_count + 1);
                cluster->power = fmaxf(cluster->power, fabsf(signal[peaks[j]]));
                cluster->sample_count++;
                processed[j] = true;
            }
        }
        
        (*num_clusters)++;
    }
    
    free(processed);
    
    return 0;
}

/**
 * @brief Detect multiple targets
 */
int32_t signal_detect_multi_target(float32_t *beamformed_output,
                                   uint32_t length,
                                   uint32_t sample_rate,
                                   multi_target_result_t *result)
{
    if (beamformed_output == NULL || result == NULL || length == 0) {
        return -1;
    }
    
    result->num_targets = 0;
    result->valid = false;
    
    /* Calculate adaptive threshold */
    float32_t threshold;
    if (signal_adaptive_threshold(beamformed_output, length, &threshold) != 0) {
        return -2;
    }
    
    /* Find peaks */
    uint32_t peaks[MAX_TARGETS * 2];
    uint32_t num_peaks;
    if (signal_find_peaks(beamformed_output, length, threshold, 
                         peaks, MAX_TARGETS * 2, &num_peaks) != 0) {
        return -3;
    }
    
    if (num_peaks == 0) {
        return 0;  // No targets found
    }
    
    /* Cluster detections */
    target_cluster_t clusters[MAX_TARGETS];
    uint32_t num_clusters;
    if (signal_cluster_detections(peaks, num_peaks, beamformed_output, length,
                                  sample_rate, clusters, MAX_TARGETS, 
                                  &num_clusters) != 0) {
        return -4;
    }
    
    /* Copy to result */
    for (uint32_t i = 0; i < num_clusters && i < MAX_TARGETS; i++) {
        result->targets[i] = clusters[i];
        result->targets[i].azimuth_rad = 0.0f;  // Would need beamforming info
        result->targets[i].elevation_rad = 0.0f;
    }
    
    result->num_targets = num_clusters;
    result->valid = (num_clusters > 0);
    
    return 0;
}

/**
 * @brief Apply bandpass filter (simplified IIR)
 */
int32_t signal_bandpass_filter(float32_t *input,
                              float32_t *output,
                              uint32_t length,
                              float32_t low_freq,
                              float32_t high_freq,
                              uint32_t sample_rate)
{
    if (input == NULL || output == NULL || length == 0) {
        return -1;
    }
    
    /* Simple moving average as placeholder */
    /* In practice, use arm_biquad_cascade_df1_f32 or similar */
    uint32_t window_size = sample_rate / (high_freq - low_freq);
    if (window_size > length) {
        window_size = length;
    }
    
    for (uint32_t i = 0; i < length; i++) {
        float32_t sum = 0.0f;
        uint32_t count = 0;
        
        for (uint32_t j = (i > window_size/2) ? i - window_size/2 : 0;
             j < length && j < i + window_size/2;
             j++) {
            sum += input[j];
            count++;
        }
        
        output[i] = (count > 0) ? sum / count : input[i];
    }
    
    return 0;
}

/**
 * @brief Calculate signal statistics
 */
int32_t signal_calculate_stats(float32_t *signal,
                              uint32_t length,
                              float32_t *mean,
                              float32_t *std,
                              float32_t *peak)
{
    if (signal == NULL || length == 0) {
        return -1;
    }
    
    if (mean != NULL) {
        arm_mean_f32(signal, length, mean);
    }
    
    if (std != NULL) {
        arm_std_f32(signal, length, std);
    }
    
    if (peak != NULL) {
        float32_t max_val;
        uint32_t max_idx;
        arm_max_f32(signal, length, &max_val, &max_idx);
        *peak = max_val;
    }
    
    return 0;
}

/**
 * @brief Normalize signal
 */
void signal_normalize(float32_t *signal, uint32_t length)
{
    if (signal == NULL || length == 0) {
        return;
    }
    
    float32_t max_val;
    uint32_t max_idx;
    arm_max_f32(signal, length, &max_val, &max_idx);
    
    if (max_val > 0.0f) {
        arm_scale_f32(signal, 1.0f / max_val, signal, length);
    }
}

/**
 * @brief Apply Hanning window
 */
void signal_apply_window(float32_t *signal, uint32_t length)
{
    if (signal == NULL || length == 0) {
        return;
    }
    
    for (uint32_t i = 0; i < length; i++) {
        float32_t window = 0.5f * (1.0f - arm_cos_f32(2.0f * M_PI * i / (length - 1)));
        signal[i] *= window;
    }
}

