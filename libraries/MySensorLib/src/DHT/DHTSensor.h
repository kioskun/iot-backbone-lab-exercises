#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include "Sensor.h"
#include <DHT.h>

/**
 * @brief DHT temperature and humidity sensor implementation
 * 
 * This class implements the Sensor interface for DHT11/DHT22 sensors.
 * It provides temperature and humidity readings.
 */
class DHTSensor : public Sensor {
public:
    /**
     * @brief Construct a new DHTSensor object
     * 
     * @param pin GPIO pin connected to the DHT data line
     * @param type Sensor type (DHT11, DHT22, etc.)
     */
    DHTSensor(uint8_t pin, uint8_t type);
    
    /**
     * @brief Initialize the DHT sensor
     */
    void begin() override;
    
    /**
     * @brief Update temperature and humidity readings
     */
    void update() override;
    
    /**
     * @brief Get the sensor name
     * 
     * @return String "DHT"
     */
    String getName() const override;
    
    /**
     * @brief Get a formatted string for display on OLED
     * 
     * @return String Formatted string with temperature and humidity
     */
    String getDisplayString() const override;
    
    /**
     * @brief Get JSON fields for this sensor
     * 
     * @return String JSON-formatted string with temperature and humidity data
     */
    String getJSONFields() const override;
    
    /**
     * @brief Get the current temperature
     * 
     * @return float Temperature in degrees Celsius
     */
    float getTemperature() const;
    
    /**
     * @brief Get the current humidity
     * 
     * @return float Humidity in percentage
     */
    float getHumidity() const;
    
    /**
     * @brief Reset sensor readings to NaN
     */
    void reset() override;
    
private:
    DHT dht;           ///< DHT sensor instance
    float temperature; ///< Last read temperature value
    float humidity;    ///< Last read humidity value
};

#endif // DHT_SENSOR_H
