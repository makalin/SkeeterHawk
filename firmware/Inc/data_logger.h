/**
 ******************************************************************************
 * @file    data_logger.h
 * @brief   Data logging utilities for debugging and analysis
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "sonar.h"
#include "guidance.h"

/* Logger Configuration */
#define LOGGER_BUFFER_SIZE        10000U
#define LOGGER_MAX_ENTRIES        1000U

/* Log Entry Types */
typedef enum {
    LOG_ENTRY_SONAR_DATA = 0,
    LOG_ENTRY_TARGET_DETECTION,
    LOG_ENTRY_GUIDANCE_CMD,
    LOG_ENTRY_VEHICLE_STATE,
    LOG_ENTRY_DIAGNOSTIC,
    LOG_ENTRY_MAX
} log_entry_type_t;

/* Sonar Data Log Entry */
typedef struct {
    uint32_t timestamp_ms;
    float32_t *rx_data[NUM_MICS];
    uint32_t sample_count;
    float32_t *beamformed_output;
    uint32_t beamformed_length;
} log_entry_sonar_t;

/* Target Detection Log Entry */
typedef struct {
    uint32_t timestamp_ms;
    target_info_t target;
    float confidence;
} log_entry_target_t;

/* Guidance Command Log Entry */
typedef struct {
    uint32_t timestamp_ms;
    guidance_cmd_t cmd;
    vehicle_state_t vehicle_state;
} log_entry_guidance_t;

/* Generic Log Entry */
typedef struct {
    log_entry_type_t type;
    union {
        log_entry_sonar_t sonar;
        log_entry_target_t target;
        log_entry_guidance_t guidance;
    } data;
} log_entry_t;

/* Logger State */
typedef struct {
    log_entry_t entries[LOGGER_MAX_ENTRIES];
    uint32_t write_index;
    uint32_t read_index;
    uint32_t entry_count;
    bool enabled;
    bool overflow;
} data_logger_t;

/* Function Prototypes */

/**
 * @brief Initialize data logger
 * @param logger Pointer to logger structure
 * @return 0 on success, negative on error
 */
int32_t logger_init(data_logger_t *logger);

/**
 * @brief Enable/disable logging
 * @param logger Pointer to logger structure
 * @param enable Enable flag
 */
void logger_set_enabled(data_logger_t *logger, bool enable);

/**
 * @brief Log sonar data
 * @param logger Pointer to logger structure
 * @param rx_data Received signals
 * @param sample_count Number of samples
 * @param beamformed Beamformed output
 * @param beamformed_length Length of beamformed data
 * @return 0 on success, negative on error
 */
int32_t logger_log_sonar(data_logger_t *logger,
                         float32_t *rx_data[NUM_MICS],
                         uint32_t sample_count,
                         float32_t *beamformed,
                         uint32_t beamformed_length);

/**
 * @brief Log target detection
 * @param logger Pointer to logger structure
 * @param target Detected target information
 * @param confidence Detection confidence
 * @return 0 on success, negative on error
 */
int32_t logger_log_target(data_logger_t *logger,
                         target_info_t *target,
                         float confidence);

/**
 * @brief Log guidance command
 * @param logger Pointer to logger structure
 * @param cmd Guidance command
 * @param vehicle_state Current vehicle state
 * @return 0 on success, negative on error
 */
int32_t logger_log_guidance(data_logger_t *logger,
                           guidance_cmd_t *cmd,
                           vehicle_state_t *vehicle_state);

/**
 * @brief Get number of logged entries
 * @param logger Pointer to logger structure
 * @return Number of entries
 */
uint32_t logger_get_count(data_logger_t *logger);

/**
 * @brief Read log entry
 * @param logger Pointer to logger structure
 * @param entry Output log entry
 * @return 0 on success, negative if no entries
 */
int32_t logger_read(data_logger_t *logger, log_entry_t *entry);

/**
 * @brief Clear all log entries
 * @param logger Pointer to logger structure
 */
void logger_clear(data_logger_t *logger);

/**
 * @brief Export logs to UART (for debugging)
 * @param logger Pointer to logger structure
 * @return 0 on success, negative on error
 */
int32_t logger_export_uart(data_logger_t *logger);

#endif /* DATA_LOGGER_H */

