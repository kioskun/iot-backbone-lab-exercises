#include "DHTSensor.h"

/**
 * @brief Construct a new DHTSensor object
 * 
 * @param pin GPIO pin connected to the DHT data line
 * @param type Sensor type (DHT11, DHT22, etc.)
 */
DHTSensor::DHTSensor(uint8_t pin, uint8_t type) : 
    dht(pin, type),
    temperature(0.0f),
    humidity(0.0f) {
}

/**
 * @brief Initialize the DHT sensor
 */
void DHTSensor::begin() {
    dht.begin();
    // Wait a bit for the sensor to stabilize
    delay(100);
}

/**
 * @brief Update temperature and humidity readings
 */
void DHTSensor::update() {
    // Read humidity
    humidity = dht.readHumidity();
    
    // Read temperature in Celsius
    temperature = dht.readTemperature();
    
    // Check if any reads failed and exit early (to try again)
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    
}

/**
 * @brief Get the sensor name
 * 
 * @return String "DHT"
 */
String DHTSensor::getName() const {
    return "DHT";
}

/**
 * @brief Get a formatted string for display on OLED
 * 
 * @return String Formatted string with temperature and humidity
 */
String DHTSensor::getDisplayString() const {
    return "DHT: T=" + String(temperature, 1) + "C H=" + String(humidity, 0) + "%";
}

/**
 * @brief Get JSON fields for this sensor
 * 
 * @return String JSON-formatted string with temperature and humidity data
 */
String DHTSensor::getJSONFields() const {
    return "\"DHT_Temp\":" + String(temperature, 1) + 
           ",\"DHT_Hum\":" + String(humidity, 0);
}

/**
 * @brief Get the current temperature
 * 
 * @return float Temperature in degrees Celsius
 */
float DHTSensor::getTemperature() const {
    return temperature;
}

/**
 * @brief Get the current humidity
 * 
 * @return float Humidity in percentage
 */
float DHTSensor::getHumidity() const {
    return humidity;
}

/**
 * @brief Reset sensor readings to indicate sensor is off
 */
void DHTSensor::reset() {
    temperature = NAN;
    humidity = NAN;
}
