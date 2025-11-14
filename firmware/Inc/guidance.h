/**
 ******************************************************************************
 * @file    guidance.h
 * @brief   Proportional Navigation guidance law for target intercept
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef GUIDANCE_H
#define GUIDANCE_H

#include <stdint.h>
#include <stdbool.h>
#include "sonar.h"

/* Guidance Parameters */
#define GUIDANCE_N               3.0f          // Navigation constant
#define GUIDANCE_MAX_ACCEL       9.81f         // Max acceleration (m/s²)
#define GUIDANCE_MIN_RANGE_CM    5.0f          // Minimum intercept range

/* Vehicle State */
typedef struct {
    float pos_x;                // Position X (m)
    float pos_y;                // Position Y (m)
    float pos_z;                // Position Z (m)
    float vel_x;                // Velocity X (m/s)
    float vel_y;                // Velocity Y (m/s)
    float vel_z;                // Velocity Z (m/s)
} vehicle_state_t;

/* Guidance Command */
typedef struct {
    float accel_x;              // Acceleration command X (m/s²)
    float accel_y;              // Acceleration command Y (m/s²)
    float accel_z;              // Acceleration command Z (m/s²)
    bool intercept;             // Intercept flag
} guidance_cmd_t;

/* Function Prototypes */

/**
 * @brief Initialize guidance system
 */
void guidance_init(void);

/**
 * @brief Compute proportional navigation guidance command
 * @param vehicle Current vehicle state
 * @param target Detected target information
 * @param cmd Output guidance command
 * @return 0 on success, negative on error
 */
int32_t guidance_compute(vehicle_state_t *vehicle, 
                         target_info_t *target, 
                         guidance_cmd_t *cmd);

/**
 * @brief Convert guidance command to motor commands
 * @param cmd Guidance command
 * @param motor_thrust Output motor thrust values (0-1.0)
 */
void guidance_to_motors(guidance_cmd_t *cmd, float *motor_thrust);

#endif /* GUIDANCE_H */

