#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

/**
 * @brief Abstract base class for all sensors in the system
 * 
 * This class defines the interface that all sensor implementations must follow.
 * It provides common functionality and properties for all sensors.
 */
class Sensor {
public:
    /**
     * @brief Default constructor
     */
    Sensor() : active(false) {}
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Sensor() {}
    
    /**
     * @brief Initialize the sensor
     * 
     * This method should be called once during setup to initialize the sensor hardware.
     */
    virtual void begin() = 0;
    
    /**
     * @brief Update sensor readings
     * 
     * This method should be called periodically to read new values from the sensor.
     */
    virtual void update() = 0;
    
    /**
     * @brief Get the sensor name
     * 
     * @return String The name of the sensor
     */
    virtual String getName() const = 0;
    
    /**
     * @brief Get a formatted string for display on OLED
     * 
     * @return String Formatted string with sensor readings
     */
    virtual String getDisplayString() const = 0;
    
    /**
     * @brief Get JSON fields for this sensor
     * 
     * @return String JSON-formatted string with sensor data fields
     */
    virtual String getJSONFields() const = 0;
    
    /**
     * @brief Reset the sensor data when disabled
     * 
     * This method should clear stored sensor readings so that no data is read
     * or displayed when the sensor is disabled.
     */
    virtual void reset() = 0;
    
    /**
     * @brief Flag indicating if the sensor is active
     * 
     * When false, the sensor will not be updated or included in data transmissions.
     */
    bool active;
};

#endif // SENSOR_H
