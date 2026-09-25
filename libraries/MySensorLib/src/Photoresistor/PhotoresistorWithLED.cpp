#include "PhotoresistorWithLED.h"

/**
 * @brief Construct a new PhotoresistorWithLED object
 * 
 * @param sensorPin GPIO pin connected to the photoresistor
 * @param ledPin GPIO pin connected to the LED
 * @param threshold Light level threshold for LED control (0-4095)
 */
PhotoresistorWithLED::PhotoresistorWithLED(uint8_t sensorPin, uint8_t ledPin, uint16_t threshold) :
    sensorPin(sensorPin),
    ledPin(ledPin),
    threshold(threshold),
    lightLevel(0),
    ledState(false) {
}

/**
 * @brief Initialize the photoresistor and LED pins
 */
void PhotoresistorWithLED::begin() {
    // Configure pins
    pinMode(sensorPin, INPUT);
    pinMode(ledPin, OUTPUT);
    
    // Initialize LED to off state
    digitalWrite(ledPin, LOW);
    ledState = false;
}

/**
 * @brief Update light level readings and control LED
 */
void PhotoresistorWithLED::update() {
    // Read analog value from photoresistor
    lightLevel = analogRead(sensorPin);
    
    // Control LED based on light level and threshold
    // If light level is below threshold (darker), turn on LED
    // If light level is above threshold (brighter), turn off LED
    if (lightLevel < threshold && !ledState) {
        digitalWrite(ledPin, HIGH);
        ledState = true;
        Serial.println("LED turned ON due to low light level");
    } else if (lightLevel >= threshold && ledState) {
        digitalWrite(ledPin, LOW);
        ledState = false;
        Serial.println("LED turned OFF due to high light level");
    }
    
}

/**
 * @brief Get the sensor name
 * 
 * @return String "Photoresistor"
 */
String PhotoresistorWithLED::getName() const {
    return "Photoresistor";
}

/**
 * @brief Get a formatted string for display on OLED
 * 
 * @return String Formatted string with light level and LED status
 */
String PhotoresistorWithLED::getDisplayString() const {
    return "Light: " + String(lightLevel) + " LED: " + (ledState ? "ON" : "OFF");
}

/**
 * @brief Get JSON fields for this sensor
 * 
 * @return String JSON-formatted string with light level and LED status
 */
String PhotoresistorWithLED::getJSONFields() const {
    return "\"Photo\":" + String(lightLevel) + 
           ",\"LED\":" + (ledState ? "1" : "0");
}

/**
 * @brief Get the current light level
 * 
 * @return int16_t Light level (0-4095, or -1 if disabled)
 */
int16_t PhotoresistorWithLED::getLightLevel() const {
    return lightLevel;
}

/**
 * @brief Get the current LED state
 * 
 * @return bool true if LED is on, false if off
 */
bool PhotoresistorWithLED::getLEDState() const {
    return ledState;
}

/**
 * @brief Set the threshold for LED control
 * 
 * @param threshold Light level threshold (0-4095)
 */
void PhotoresistorWithLED::setThreshold(uint16_t threshold) {
    this->threshold = threshold;
}

/**
 * @brief Reset sensor readings to indicate sensor is off
 */
void PhotoresistorWithLED::reset() {
    lightLevel = -1;            // Set to -1 to indicate no valid reading
    // Ensure LED stays off
    digitalWrite(ledPin, LOW);
    ledState = false;
}
