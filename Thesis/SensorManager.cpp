#include "SensorManager.h"
#include "DHT/DHTSensor.h"
#include "Photoresistor/PhotoresistorWithLED.h"

// Global instance
SensorManager sensorManager;

/**
 * @brief Default constructor
 */
SensorManager::SensorManager() {
}

/**
 * @brief Initialize the sensor manager and register sensors
 * 
 * This function creates sensor instances and registers them with the manager.
 * It's called during system initialization.
 */
void initSensorManager() {
  // Create static instances of sensors
  // DHT11 sensor on GPIO13
  static DHTSensor dht(13, DHT11);
  
  // Photoresistor on GPIO36 with LED on GPIO17
  static PhotoresistorWithLED photo(36, 17);
  
  // Register sensors with the manager
  sensorManager.addSensor(&dht);
  sensorManager.addSensor(&photo);
  
  // Initialize all sensors
  sensorManager.begin();
}

/**
 * @brief FreeRTOS task function for reading sensors
 * 
 * This task periodically updates all active sensors.
 * 
 * @param pvParameters Task parameters (unused)
 */
void sensorTask(void *pvParameters) {
  (void) pvParameters;
  
  // Ensure sensors are initialized
  sensorManager.begin();
  
  // Task loop
  for (;;) {
    // Update all active sensors
    sensorManager.update();
    
    // Delay until next reading cycle (2.5 seconds)
    vTaskDelay(pdMS_TO_TICKS(2500));
  }
}

/**
 * @brief Add a sensor to the manager
 * 
 * @param sensor Pointer to a Sensor object
 */
void SensorManager::addSensor(Sensor* sensor) {
  sensors.push_back(sensor);
}

/**
 * @brief Initialize all sensors
 * 
 * This method calls begin() on all registered sensors.
 */
void SensorManager::begin() {
  for (auto s : sensors) {
    s->begin();
  }
}

/**
 * @brief Update all active sensors
 * 
 * This method calls update() on all active sensors.
 */
void SensorManager::update() {
  for (auto s : sensors) {
    if (s->active) {
      s->update();
    }
  }
}

/**
 * @brief Enable a sensor by name
 * 
 * @param name Name of the sensor to enable
 */
void SensorManager::enableSensor(const String &name) {
  for (auto s : sensors) {
    if (s->getName() == name) {
      s->active = true;
    }
  }
}

/**
 * @brief Disable a sensor by name
 * 
 * @param name Name of the sensor to disable
 */
void SensorManager::disableSensor(const String &name) {
  for (auto s : sensors) {
    if (s->getName() == name) {
      s->active = false;
      s->reset(); // Reset the sensor data immediately so that it stops reading and clears its values.
    }
  }
}

/**
 * @brief Get JSON representation of all active sensor data
 * 
 * @return String JSON-formatted string with all active sensor data
 */
String SensorManager::getSensorDataJSON() {
  String json = "{";
  bool first = true;

  for (auto s : sensors) {
    if (!s->active) continue; // only output active sensors

    // Each sensor returns its snippet. Might be 1 field or multiple.
    String snippet = s->getJSONFields();

    if (!first) {
      json += ",";
    }
    json += snippet;
    first = false;
  }

  json += "}";
  return json;
}
