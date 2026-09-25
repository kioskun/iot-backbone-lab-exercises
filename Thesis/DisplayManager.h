#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <freertos/semphr.h>

/**
 * @brief Display manager for OLED screen
 * 
 * This module manages the OLED display, showing system status
 * and sensor readings.
 */

/**
 * @brief Initialize the display manager
 * 
 * This function initializes the OLED display and I2C communication.
 */
void initDisplayManager();

/**
 * @brief FreeRTOS task function for updating the display
 * 
 * This task periodically updates the OLED display with
 * current system status and sensor readings.
 * 
 * @param pvParameters Task parameters (unused)
 */
void displayTask(void *pvParameters);

/**
 * @brief Mutex for I2C bus access
 * 
 * This mutex protects I2C bus access when multiple tasks
 * need to use I2C devices.
 */
extern SemaphoreHandle_t i2cMutex;

#endif // DISPLAY_MANAGER_H
