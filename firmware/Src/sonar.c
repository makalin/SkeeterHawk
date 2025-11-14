/**
 ******************************************************************************
 * @file    sonar.c
 * @brief   Active sonar processing implementation for SkeeterHawk
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "sonar.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Microphone array positions (in meters) */
static const float mic_positions[NUM_MICS][3] = {
    {-0.005f, -0.005f, 0.0f},  // Mic 0: bottom-left
    { 0.005f, -0.005f, 0.0f},  // Mic 1: bottom-right
    {-0.005f,  0.005f, 0.0f},  // Mic 2: top-left
    { 0.005f,  0.005f, 0.0f}   // Mic 3: top-right
};

/* Speed of sound at 20°C (m/s) */
#define SPEED_OF_SOUND 343.0f

/**
 * @brief Initialize sonar system
 */
int32_t sonar_init(sonar_state_t *state)
{
    if (state == NULL) {
        return -1;
    }
    
    /* Allocate memory for buffers */
    state->tx_chirp = (float32_t *)malloc(SONAR_CHIRP_SAMPLES * sizeof(float32_t));
    state->matched_filter = (float32_t *)malloc(SONAR_CHIRP_SAMPLES * sizeof(float32_t));
    state->beamformed_output = (float32_t *)malloc(SONAR_MAX_SAMPLES * sizeof(float32_t));
    
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        state->rx_buffer[i] = (float32_t *)malloc(SONAR_MAX_SAMPLES * sizeof(float32_t));
        /* Filtered buffer needs to be larger for correlation output */
        state->filtered_buffer[i] = (float32_t *)malloc((SONAR_MAX_SAMPLES + SONAR_CHIRP_SAMPLES) * sizeof(float32_t));
        
        if (state->rx_buffer[i] == NULL || state->filtered_buffer[i] == NULL) {
            return -2;
        }
    }
    
    if (state->tx_chirp == NULL || state->matched_filter == NULL || 
        state->beamformed_output == NULL) {
        return -2;
    }
    
    /* Generate transmit chirp */
    sonar_generate_chirp(state->tx_chirp, SONAR_CHIRP_SAMPLES);
    
    /* Create matched filter (time-reversed chirp) */
    for (uint32_t i = 0; i < SONAR_CHIRP_SAMPLES; i++) {
        state->matched_filter[i] = state->tx_chirp[SONAR_CHIRP_SAMPLES - 1 - i];
    }
    
    /* Initialize state */
    memset(&state->target, 0, sizeof(target_info_t));
    state->processing_complete = false;
    state->sample_count = 0;
    
    return 0;
}

/**
 * @brief Generate transmit chirp signal
 */
void sonar_generate_chirp(float32_t *chirp, uint32_t length)
{
    const float32_t fs = (float32_t)SONAR_SAMPLE_RATE;
    const float32_t duration = (float32_t)length / fs;
    const float32_t f0 = (float32_t)SONAR_CHIRP_F0;
    const float32_t f1 = (float32_t)SONAR_CHIRP_F1;
    const float32_t bandwidth = f1 - f0;
    const float32_t chirp_rate = bandwidth / duration;
    
    /* Generate LFM chirp */
    for (uint32_t i = 0; i < length; i++) {
        float32_t t = (float32_t)i / fs;
        float32_t phase = 2.0f * M_PI * (f0 * t + 0.5f * chirp_rate * t * t);
        chirp[i] = arm_cos_f32(phase);
        
        /* Apply Hanning window */
        float32_t window = 0.5f * (1.0f - arm_cos_f32(2.0f * M_PI * i / (length - 1)));
        chirp[i] *= window;
    }
}

/**
 * @brief Process received signals with matched filter
 */
void sonar_matched_filter(sonar_state_t *state, uint32_t mic_idx)
{
    if (state == NULL || mic_idx >= NUM_MICS) {
        return;
    }
    
    /* Cross-correlation (matched filtering) */
    arm_correlate_f32(
        state->rx_buffer[mic_idx],
        state->sample_count,
        state->matched_filter,
        SONAR_CHIRP_SAMPLES,
        state->filtered_buffer[mic_idx]
    );
}

/**
 * @brief Perform delay-and-sum beamforming
 */
void sonar_beamform(sonar_state_t *state, float azimuth, float elevation)
{
    if (state == NULL) {
        return;
    }
    
    /* Steering direction vector */
    float steering_dir[3] = {
        arm_cos_f32(elevation) * arm_cos_f32(azimuth),
        arm_cos_f32(elevation) * arm_sin_f32(azimuth),
        arm_sin_f32(elevation)
    };
    
    /* Calculate TDOA for each microphone */
    float delays[NUM_MICS];
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        delays[i] = (mic_positions[i][0] * steering_dir[0] +
                     mic_positions[i][1] * steering_dir[1] +
                     mic_positions[i][2] * steering_dir[2]) / SPEED_OF_SOUND;
    }
    
    /* Normalize delays relative to first mic */
    float delay_offset = delays[0];
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        delays[i] -= delay_offset;
    }
    
    /* Clear beamformed output */
    memset(state->beamformed_output, 0, state->sample_count * sizeof(float32_t));
    
    /* Delay and sum */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        int32_t delay_samples = (int32_t)(delays[i] * SONAR_SAMPLE_RATE);
        
        for (uint32_t j = 0; j < state->sample_count; j++) {
            int32_t src_idx = (int32_t)j - delay_samples;
            if (src_idx >= 0 && src_idx < (int32_t)state->sample_count) {
                state->beamformed_output[j] += state->filtered_buffer[i][src_idx];
            }
        }
    }
    
    /* Average */
    arm_scale_f32(state->beamformed_output, 1.0f / NUM_MICS, 
                  state->beamformed_output, state->sample_count);
}

/**
 * @brief Detect target using matched filtering and beamforming
 */
int32_t sonar_detect_target(sonar_state_t *state)
{
    if (state == NULL) {
        return -1;
    }
    
    /* Apply matched filter to all channels */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        sonar_matched_filter(state, i);
    }
    
    /* Search over angles */
    float max_power = 0.0f;
    float best_azimuth = 0.0f;
    float best_elevation = 0.0f;
    uint32_t best_peak_idx = 0;
    
    const float az_step = (BEAMFORM_AZIMUTH_MAX - BEAMFORM_AZIMUTH_MIN) / BEAMFORM_AZIMUTH_STEPS;
    const float el_step = (BEAMFORM_ELEVATION_MAX - BEAMFORM_ELEVATION_MIN) / BEAMFORM_ELEVATION_STEPS;
    
    for (uint32_t az_idx = 0; az_idx < BEAMFORM_AZIMUTH_STEPS; az_idx++) {
        float azimuth = BEAMFORM_AZIMUTH_MIN + az_idx * az_step;
        
        for (uint32_t el_idx = 0; el_idx < BEAMFORM_ELEVATION_STEPS; el_idx++) {
            float elevation = BEAMFORM_ELEVATION_MIN + el_idx * el_step;
            
            /* Beamform */
            sonar_beamform(state, azimuth, elevation);
            
            /* Find peak */
            float32_t max_val;
            uint32_t max_idx;
            arm_max_f32(state->beamformed_output, state->sample_count, &max_val, &max_idx);
            
            float power = fabsf(max_val);
            if (power > max_power) {
                max_power = power;
                best_azimuth = azimuth;
                best_elevation = elevation;
                best_peak_idx = max_idx;
            }
        }
    }
    
    /* Check if detection exceeds threshold */
    if (max_power < DETECTION_THRESHOLD) {
        state->target.valid = false;
        return -2;
    }
    
    /* Calculate range from peak location */
    float tof = (float32_t)best_peak_idx / SONAR_SAMPLE_RATE;
    float range_cm = (tof * SPEED_OF_SOUND * 100.0f) / 2.0f;
    
    /* Validate range */
    if (range_cm < MIN_RANGE_CM || range_cm > MAX_RANGE_CM) {
        state->target.valid = false;
        return -3;
    }
    
    /* Update target information */
    state->target.range_cm = range_cm;
    state->target.azimuth_rad = best_azimuth;
    state->target.elevation_rad = best_elevation;
    state->target.confidence = fminf(max_power / (DETECTION_THRESHOLD * 10.0f), 1.0f);
    state->target.valid = true;
    state->processing_complete = true;
    
    return 0;
}

/**
 * @brief Get detected target information
 */
int32_t sonar_get_target(sonar_state_t *state, target_info_t *target)
{
    if (state == NULL || target == NULL) {
        return -1;
    }
    
    if (!state->target.valid) {
        return -2;
    }
    
    *target = state->target;
    return 0;
}

/**
 * @brief Deinitialize sonar system
 */
void sonar_deinit(sonar_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    free(state->tx_chirp);
    free(state->matched_filter);
    free(state->beamformed_output);
    
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        free(state->rx_buffer[i]);
        free(state->filtered_buffer[i]);
    }
}

