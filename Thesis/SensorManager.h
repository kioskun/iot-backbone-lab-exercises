#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "Sensor.h"

/**
 * @brief Manager class for all sensors in the system
 * 
 * This class manages multiple sensor instances, providing centralized
 * control for initialization, updates, and data retrieval.
 */
class SensorManager {
public:
    /**
     * @brief Default constructor
     */
    SensorManager();
    
    /**
     * @brief Add a sensor to the manager
     * 
     * @param sensor Pointer to a Sensor object
     */
    void addSensor(Sensor* sensor);
    
    /**
     * @brief Initialize all sensors
     * 
     * This method calls begin() on all registered sensors.
     */
    void begin();
    
    /**
     * @brief Update all active sensors
     * 
     * This method calls update() on all active sensors.
     */
    void update();
    
    /**
     * @brief Enable a sensor by name
     * 
     * @param name Name of the sensor to enable
     */
    void enableSensor(const String &name);
    
    /**
     * @brief Disable a sensor by name
     * 
     * @param name Name of the sensor to disable
     */
    void disableSensor(const String &name);
    
    /**
     * @brief Get JSON representation of all active sensor data
     * 
     * @return String JSON-formatted string with all active sensor data
     */
    String getSensorDataJSON();
    
    /**
     * @brief Vector of all registered sensors
     */
    std::vector<Sensor*> sensors;
};

/**
 * @brief Global instance of the SensorManager
 */
extern SensorManager sensorManager;

/**
 * @brief FreeRTOS task function for reading sensors
 * 
 * @param pvParameters Task parameters (unused)
 */
void sensorTask(void *pvParameters);

/**
 * @brief Initialize the sensor manager and register sensors
 */
void initSensorManager();

#endif // SENSOR_MANAGER_H
