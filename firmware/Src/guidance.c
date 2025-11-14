/**
 ******************************************************************************
 * @file    guidance.c
 * @brief   Proportional Navigation guidance law implementation
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "guidance.h"
#include "arm_math.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Initialize guidance system
 */
void guidance_init(void)
{
    /* Nothing to initialize for now */
}

/**
 * @brief Compute proportional navigation guidance command
 */
int32_t guidance_compute(vehicle_state_t *vehicle, 
                         target_info_t *target, 
                         guidance_cmd_t *cmd)
{
    if (vehicle == NULL || target == NULL || cmd == NULL) {
        return -1;
    }
    
    if (!target->valid) {
        return -2;
    }
    
    /* Convert target from spherical to Cartesian coordinates */
    float target_range_m = target->range_cm / 100.0f;
    float target_x = target_range_m * arm_cos_f32(target->elevation_rad) * 
                     arm_cos_f32(target->azimuth_rad);
    float target_y = target_range_m * arm_cos_f32(target->elevation_rad) * 
                     arm_sin_f32(target->azimuth_rad);
    float target_z = target_range_m * arm_sin_f32(target->elevation_rad);
    
    /* Relative position */
    float rel_x = target_x - vehicle->pos_x;
    float rel_y = target_y - vehicle->pos_y;
    float rel_z = target_z - vehicle->pos_z;
    
    float range = sqrtf(rel_x * rel_x + rel_y * rel_y + rel_z * rel_z);
    
    /* Check minimum range */
    if (range < (GUIDANCE_MIN_RANGE_CM / 100.0f)) {
        cmd->intercept = true;
        cmd->accel_x = 0.0f;
        cmd->accel_y = 0.0f;
        cmd->accel_z = 0.0f;
        return 0;
    }
    
    /* Line of sight (LOS) vector */
    float los_x = rel_x / range;
    float los_y = rel_y / range;
    float los_z = rel_z / range;
    
    /* Relative velocity */
    float rel_vel_x = -vehicle->vel_x;  // Target assumed stationary
    float rel_vel_y = -vehicle->vel_y;
    float rel_vel_z = -vehicle->vel_z;
    
    /* Closing velocity */
    float closing_vel = los_x * rel_vel_x + los_y * rel_vel_y + los_z * rel_vel_z;
    
    /* LOS rate */
    float los_rate_x = (rel_vel_x - los_x * closing_vel) / range;
    float los_rate_y = (rel_vel_y - los_y * closing_vel) / range;
    float los_rate_z = (rel_vel_z - los_z * closing_vel) / range;
    
    /* Proportional Navigation: a = N * Vc * LOS_rate */
    float accel_x = GUIDANCE_N * closing_vel * los_rate_x;
    float accel_y = GUIDANCE_N * closing_vel * los_rate_y;
    float accel_z = GUIDANCE_N * closing_vel * los_rate_z;
    
    /* Limit acceleration */
    float accel_mag = sqrtf(accel_x * accel_x + accel_y * accel_y + accel_z * accel_z);
    if (accel_mag > GUIDANCE_MAX_ACCEL) {
        float scale = GUIDANCE_MAX_ACCEL / accel_mag;
        accel_x *= scale;
        accel_y *= scale;
        accel_z *= scale;
    }
    
    cmd->accel_x = accel_x;
    cmd->accel_y = accel_y;
    cmd->accel_z = accel_z;
    cmd->intercept = false;
    
    return 0;
}

/**
 * @brief Convert guidance command to motor commands
 * Simplified quadcopter control mixing
 */
void guidance_to_motors(guidance_cmd_t *cmd, float *motor_thrust)
{
    if (cmd == NULL || motor_thrust == NULL) {
        return;
    }
    
    /* Base hover thrust (adjust based on vehicle weight) */
    float base_thrust = 0.5f;
    
    /* Convert acceleration to motor commands */
    /* Simplified mixing for X-configuration quadcopter */
    motor_thrust[0] = base_thrust + cmd->accel_x * 0.25f + cmd->accel_y * 0.25f + cmd->accel_z * 0.25f;  // Front-left
    motor_thrust[1] = base_thrust - cmd->accel_x * 0.25f + cmd->accel_y * 0.25f + cmd->accel_z * 0.25f;  // Front-right
    motor_thrust[2] = base_thrust + cmd->accel_x * 0.25f - cmd->accel_y * 0.25f + cmd->accel_z * 0.25f;  // Rear-left
    motor_thrust[3] = base_thrust - cmd->accel_x * 0.25f - cmd->accel_y * 0.25f + cmd->accel_z * 0.25f;  // Rear-right
    
    /* Clamp to valid range */
    for (int i = 0; i < 4; i++) {
        if (motor_thrust[i] < 0.0f) motor_thrust[i] = 0.0f;
        if (motor_thrust[i] > 1.0f) motor_thrust[i] = 1.0f;
    }
}

