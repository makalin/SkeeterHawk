/**
 ******************************************************************************
 * @file    config.c
 * @brief   Configuration management implementation
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "config.h"
#include <string.h>

/**
 * @brief Initialize configuration with defaults
 */
int32_t config_init(system_config_t *config)
{
    if (config == NULL) {
        return -1;
    }
    
    config->version = CONFIG_VERSION;
    
    /* Initialize sonar config */
    config->sonar.sample_rate = CONFIG_DEFAULT_SONAR_SAMPLE_RATE;
    config->sonar.chirp_duration_ms = CONFIG_DEFAULT_SONAR_CHIRP_DURATION_MS;
    config->sonar.chirp_f0 = CONFIG_DEFAULT_SONAR_CHIRP_F0;
    config->sonar.chirp_f1 = CONFIG_DEFAULT_SONAR_CHIRP_F1;
    config->sonar.detection_threshold = CONFIG_DEFAULT_SONAR_THRESHOLD;
    config->sonar.max_range_cm = CONFIG_DEFAULT_SONAR_MAX_RANGE_CM;
    config->sonar.min_range_cm = CONFIG_DEFAULT_SONAR_MIN_RANGE_CM;
    
    /* Initialize guidance config */
    config->guidance.navigation_constant = CONFIG_DEFAULT_GUIDANCE_N;
    config->guidance.max_acceleration = CONFIG_DEFAULT_GUIDANCE_MAX_ACCEL;
    config->guidance.min_range_cm = CONFIG_DEFAULT_GUIDANCE_MIN_RANGE_CM;
    
    /* Initialize system config */
    config->logging_enabled = false;
    config->log_level = 2;  // Warnings
    
    return 0;
}

/**
 * @brief Load configuration from non-volatile memory
 */
int32_t config_load(system_config_t *config)
{
    if (config == NULL) {
        return -1;
    }
    
    /* Placeholder - would read from flash/EEPROM */
    /* In practice, use HAL_FLASH_Read or similar */
    
    /* For now, just initialize with defaults */
    return config_init(config);
}

/**
 * @brief Save configuration to non-volatile memory
 */
int32_t config_save(system_config_t *config)
{
    if (config == NULL) {
        return -1;
    }
    
    /* Validate before saving */
    if (config_validate(config) != 0) {
        return -2;
    }
    
    /* Placeholder - would write to flash/EEPROM */
    /* In practice, use HAL_FLASH_Program or similar */
    
    return 0;
}

/**
 * @brief Validate configuration
 */
int32_t config_validate(system_config_t *config)
{
    if (config == NULL) {
        return -1;
    }
    
    /* Validate sonar config */
    if (config->sonar.sample_rate < 100000 || config->sonar.sample_rate > 500000) {
        return -2;
    }
    if (config->sonar.chirp_duration_ms == 0 || config->sonar.chirp_duration_ms > 10) {
        return -3;
    }
    if (config->sonar.chirp_f0 >= config->sonar.chirp_f1) {
        return -4;
    }
    if (config->sonar.min_range_cm >= config->sonar.max_range_cm) {
        return -5;
    }
    
    /* Validate guidance config */
    if (config->guidance.navigation_constant < 1.0f || 
        config->guidance.navigation_constant > 10.0f) {
        return -6;
    }
    if (config->guidance.max_acceleration <= 0.0f) {
        return -7;
    }
    
    return 0;
}

/**
 * @brief Reset configuration to defaults
 */
int32_t config_reset(system_config_t *config)
{
    return config_init(config);
}

/**
 * @brief Get sonar configuration
 */
int32_t config_get_sonar(system_config_t *config, sonar_config_t *sonar_config)
{
    if (config == NULL || sonar_config == NULL) {
        return -1;
    }
    
    *sonar_config = config->sonar;
    return 0;
}

/**
 * @brief Set sonar configuration
 */
int32_t config_set_sonar(system_config_t *config, sonar_config_t *sonar_config)
{
    if (config == NULL || sonar_config == NULL) {
        return -1;
    }
    
    config->sonar = *sonar_config;
    
    /* Validate */
    return config_validate(config);
}

/**
 * @brief Get guidance configuration
 */
int32_t config_get_guidance(system_config_t *config, guidance_config_t *guidance_config)
{
    if (config == NULL || guidance_config == NULL) {
        return -1;
    }
    
    *guidance_config = config->guidance;
    return 0;
}

/**
 * @brief Set guidance configuration
 */
int32_t config_set_guidance(system_config_t *config, guidance_config_t *guidance_config)
{
    if (config == NULL || guidance_config == NULL) {
        return -1;
    }
    
    config->guidance = *guidance_config;
    
    /* Validate */
    return config_validate(config);
}

