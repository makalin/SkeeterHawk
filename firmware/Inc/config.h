/**
 ******************************************************************************
 * @file    config.h
 * @brief   Configuration management for SkeeterHawk
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* Configuration Version */
#define CONFIG_VERSION             1U

/* Sonar Configuration */
typedef struct {
    uint32_t sample_rate;          // Sample rate in Hz
    uint32_t chirp_duration_ms;     // Chirp duration in ms
    uint32_t chirp_f0;             // Chirp start frequency (Hz)
    uint32_t chirp_f1;             // Chirp end frequency (Hz)
    float detection_threshold;      // Detection threshold
    uint32_t max_range_cm;          // Maximum detection range (cm)
    uint32_t min_range_cm;          // Minimum detection range (cm)
} sonar_config_t;

/* Guidance Configuration */
typedef struct {
    float navigation_constant;      // Proportional navigation constant
    float max_acceleration;        // Maximum acceleration (m/s²)
    float min_range_cm;             // Minimum intercept range (cm)
} guidance_config_t;

/* System Configuration */
typedef struct {
    uint32_t version;
    sonar_config_t sonar;
    guidance_config_t guidance;
    bool logging_enabled;
    uint32_t log_level;             // 0=off, 1=errors, 2=warnings, 3=info, 4=debug
} system_config_t;

/* Default Configurations */
#define CONFIG_DEFAULT_SONAR_SAMPLE_RATE        200000U
#define CONFIG_DEFAULT_SONAR_CHIRP_DURATION_MS  1U
#define CONFIG_DEFAULT_SONAR_CHIRP_F0           38000U
#define CONFIG_DEFAULT_SONAR_CHIRP_F1           42000U
#define CONFIG_DEFAULT_SONAR_THRESHOLD          1000.0f
#define CONFIG_DEFAULT_SONAR_MAX_RANGE_CM       500U
#define CONFIG_DEFAULT_SONAR_MIN_RANGE_CM       10U

#define CONFIG_DEFAULT_GUIDANCE_N               3.0f
#define CONFIG_DEFAULT_GUIDANCE_MAX_ACCEL       9.81f
#define CONFIG_DEFAULT_GUIDANCE_MIN_RANGE_CM    5.0f

/* Function Prototypes */

/**
 * @brief Initialize configuration with defaults
 * @param config Pointer to configuration structure
 * @return 0 on success, negative on error
 */
int32_t config_init(system_config_t *config);

/**
 * @brief Load configuration from non-volatile memory
 * @param config Pointer to configuration structure
 * @return 0 on success, negative on error
 */
int32_t config_load(system_config_t *config);

/**
 * @brief Save configuration to non-volatile memory
 * @param config Pointer to configuration structure
 * @return 0 on success, negative on error
 */
int32_t config_save(system_config_t *config);

/**
 * @brief Validate configuration
 * @param config Pointer to configuration structure
 * @return 0 if valid, negative on error
 */
int32_t config_validate(system_config_t *config);

/**
 * @brief Reset configuration to defaults
 * @param config Pointer to configuration structure
 * @return 0 on success, negative on error
 */
int32_t config_reset(system_config_t *config);

/**
 * @brief Get sonar configuration
 * @param config System configuration
 * @param sonar_config Output sonar configuration
 * @return 0 on success, negative on error
 */
int32_t config_get_sonar(system_config_t *config, sonar_config_t *sonar_config);

/**
 * @brief Set sonar configuration
 * @param config System configuration
 * @param sonar_config Input sonar configuration
 * @return 0 on success, negative on error
 */
int32_t config_set_sonar(system_config_t *config, sonar_config_t *sonar_config);

/**
 * @brief Get guidance configuration
 * @param config System configuration
 * @param guidance_config Output guidance configuration
 * @return 0 on success, negative on error
 */
int32_t config_get_guidance(system_config_t *config, guidance_config_t *guidance_config);

/**
 * @brief Set guidance configuration
 * @param config System configuration
 * @param guidance_config Input guidance configuration
 * @return 0 on success, negative on error
 */
int32_t config_set_guidance(system_config_t *config, guidance_config_t *guidance_config);

#endif /* CONFIG_H */

