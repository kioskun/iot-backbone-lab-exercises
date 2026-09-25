#ifndef MY_SENSOR_LIB_H
#define MY_SENSOR_LIB_H

// Base sensor interface
#include "Sensor.h"

// Sensor implementations (organized in subfolders)
#include "DHT/DHTSensor.h"
#include "Photoresistor/PhotoresistorWithLED.h"

// Sensor manager for handling all sensors
#include "SensorManager.h"

// LoRaWAN module
#include "LoRaWAN.h"

// Display module
#include "DisplayManager.h"

// GUI module
#include "GUI.h"

#endif // MY_SENSOR_LIB_H
