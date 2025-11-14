/**
 ******************************************************************************
 * @file    data_logger.c
 * @brief   Data logging utilities implementation
 * @author  Mehmet T. AKALIN
 * @date    2025
 ******************************************************************************
 */

#include "data_logger.h"
#include "stm32h7xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef huart1;  // Assuming UART1 for logging

/**
 * @brief Initialize data logger
 */
int32_t logger_init(data_logger_t *logger)
{
    if (logger == NULL) {
        return -1;
    }
    
    memset(logger, 0, sizeof(data_logger_t));
    logger->enabled = false;
    logger->overflow = false;
    
    return 0;
}

/**
 * @brief Enable/disable logging
 */
void logger_set_enabled(data_logger_t *logger, bool enable)
{
    if (logger == NULL) {
        return;
    }
    
    logger->enabled = enable;
}

/**
 * @brief Log sonar data
 */
int32_t logger_log_sonar(data_logger_t *logger,
                         float32_t *rx_data[NUM_MICS],
                         uint32_t sample_count,
                         float32_t *beamformed,
                         uint32_t beamformed_length)
{
    if (logger == NULL || !logger->enabled) {
        return -1;
    }
    
    if (logger->entry_count >= LOGGER_MAX_ENTRIES) {
        logger->overflow = true;
        return -2;
    }
    
    log_entry_t *entry = &logger->entries[logger->write_index];
    entry->type = LOG_ENTRY_SONAR_DATA;
    entry->data.sonar.timestamp_ms = HAL_GetTick();
    entry->data.sonar.sample_count = sample_count;
    entry->data.sonar.beamformed_length = beamformed_length;
    
    /* Copy data pointers (assuming data persists) */
    for (uint32_t i = 0; i < NUM_MICS; i++) {
        entry->data.sonar.rx_data[i] = rx_data[i];
    }
    entry->data.sonar.beamformed_output = beamformed;
    
    logger->write_index = (logger->write_index + 1) % LOGGER_MAX_ENTRIES;
    logger->entry_count++;
    
    return 0;
}

/**
 * @brief Log target detection
 */
int32_t logger_log_target(data_logger_t *logger,
                         target_info_t *target,
                         float confidence)
{
    if (logger == NULL || !logger->enabled || target == NULL) {
        return -1;
    }
    
    if (logger->entry_count >= LOGGER_MAX_ENTRIES) {
        logger->overflow = true;
        return -2;
    }
    
    log_entry_t *entry = &logger->entries[logger->write_index];
    entry->type = LOG_ENTRY_TARGET_DETECTION;
    entry->data.target.timestamp_ms = HAL_GetTick();
    entry->data.target.target = *target;
    entry->data.target.confidence = confidence;
    
    logger->write_index = (logger->write_index + 1) % LOGGER_MAX_ENTRIES;
    logger->entry_count++;
    
    return 0;
}

/**
 * @brief Log guidance command
 */
int32_t logger_log_guidance(data_logger_t *logger,
                           guidance_cmd_t *cmd,
                           vehicle_state_t *vehicle_state)
{
    if (logger == NULL || !logger->enabled || cmd == NULL || vehicle_state == NULL) {
        return -1;
    }
    
    if (logger->entry_count >= LOGGER_MAX_ENTRIES) {
        logger->overflow = true;
        return -2;
    }
    
    log_entry_t *entry = &logger->entries[logger->write_index];
    entry->type = LOG_ENTRY_GUIDANCE_CMD;
    entry->data.guidance.timestamp_ms = HAL_GetTick();
    entry->data.guidance.cmd = *cmd;
    entry->data.guidance.vehicle_state = *vehicle_state;
    
    logger->write_index = (logger->write_index + 1) % LOGGER_MAX_ENTRIES;
    logger->entry_count++;
    
    return 0;
}

/**
 * @brief Get number of logged entries
 */
uint32_t logger_get_count(data_logger_t *logger)
{
    if (logger == NULL) {
        return 0;
    }
    
    return logger->entry_count;
}

/**
 * @brief Read log entry
 */
int32_t logger_read(data_logger_t *logger, log_entry_t *entry)
{
    if (logger == NULL || entry == NULL) {
        return -1;
    }
    
    if (logger->entry_count == 0) {
        return -2;
    }
    
    *entry = logger->entries[logger->read_index];
    logger->read_index = (logger->read_index + 1) % LOGGER_MAX_ENTRIES;
    logger->entry_count--;
    
    return 0;
}

/**
 * @brief Clear all log entries
 */
void logger_clear(data_logger_t *logger)
{
    if (logger == NULL) {
        return;
    }
    
    memset(logger, 0, sizeof(data_logger_t));
    logger->enabled = false;
    logger->overflow = false;
}

/**
 * @brief Export logs to UART
 */
int32_t logger_export_uart(data_logger_t *logger)
{
    if (logger == NULL) {
        return -1;
    }
    
    char buffer[256];
    uint32_t count = logger->entry_count;
    uint32_t read_idx = logger->read_index;
    
    for (uint32_t i = 0; i < count; i++) {
        log_entry_t *entry = &logger->entries[read_idx];
        
        switch (entry->type) {
            case LOG_ENTRY_TARGET_DETECTION:
                snprintf(buffer, sizeof(buffer),
                        "T=%lu: Target: R=%.2fcm, Az=%.2f°, El=%.2f°, Conf=%.2f\n",
                        entry->data.target.timestamp_ms,
                        entry->data.target.target.range_cm,
                        entry->data.target.target.azimuth_rad * 57.3f,
                        entry->data.target.target.elevation_rad * 57.3f,
                        entry->data.target.confidence);
                HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
                break;
                
            case LOG_ENTRY_GUIDANCE_CMD:
                snprintf(buffer, sizeof(buffer),
                        "T=%lu: Guidance: Accel=(%.2f,%.2f,%.2f) m/s²\n",
                        entry->data.guidance.timestamp_ms,
                        entry->data.guidance.cmd.accel_x,
                        entry->data.guidance.cmd.accel_y,
                        entry->data.guidance.cmd.accel_z);
                HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
                break;
                
            default:
                break;
        }
        
        read_idx = (read_idx + 1) % LOGGER_MAX_ENTRIES;
    }
    
    return 0;
}

