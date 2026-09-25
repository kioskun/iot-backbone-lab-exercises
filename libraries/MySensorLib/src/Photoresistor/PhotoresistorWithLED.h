#ifndef PHOTORESISTOR_WITH_LED_H
#define PHOTORESISTOR_WITH_LED_H

#include "Sensor.h"

/**
 * @brief Photoresistor with LED control implementation
 * 
 * This class implements the Sensor interface for a photoresistor with LED control.
 * It reads light levels from a photoresistor and controls an LED based on the readings.
 */
class PhotoresistorWithLED : public Sensor {
public:
    /**
     * @brief Construct a new PhotoresistorWithLED object
     * 
     * @param sensorPin GPIO pin connected to the photoresistor
     * @param ledPin GPIO pin connected to the LED
     * @param threshold Light level threshold for LED control (0-4095)
     */
    PhotoresistorWithLED(uint8_t sensorPin, uint8_t ledPin, uint16_t threshold = 2000);
    
    /**
     * @brief Initialize the photoresistor and LED pins
     */
    void begin() override;
    
    /**
     * @brief Update light level readings and control LED
     */
    void update() override;
    
    /**
     * @brief Get the sensor name
     * 
     * @return String "Photoresistor"
     */
    String getName() const override;
    
    /**
     * @brief Get a formatted string for display on OLED
     * 
     * @return String Formatted string with light level and LED status
     */
    String getDisplayString() const override;
    
    /**
     * @brief Get JSON fields for this sensor
     * 
     * @return String JSON-formatted string with light level and LED status
     */
    String getJSONFields() const override;
    
    /**
     * @brief Get the current light level
     * 
     * @return int16_t Light level (0-4095, or -1 if disabled)
     */
    int16_t getLightLevel() const;
    
    /**
     * @brief Get the current LED state
     * 
     * @return bool true if LED is on, false if off
     */
    bool getLEDState() const;
    
    /**
     * @brief Set the threshold for LED control
     * 
     * @param threshold Light level threshold (0-4095)
     */
    void setThreshold(uint16_t threshold);
    
    /**
     * @brief Reset sensor readings to indicate sensor is off
     */
    void reset() override;
    
private:
    uint8_t sensorPin;     ///< GPIO pin for photoresistor
    uint8_t ledPin;        ///< GPIO pin for LED
    uint16_t threshold;    ///< Light level threshold for LED control
    int16_t lightLevel;    ///< Last read light level (changed to signed)
    bool ledState;         ///< Current LED state
};

#endif // PHOTORESISTOR_WITH_LED_H
