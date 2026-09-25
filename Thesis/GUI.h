#ifndef GUI_H
#define GUI_H

#include <Arduino.h>
#include <WebServer.h>

extern String g_wifiMode;  // "AP" or "Station"
extern String g_wifiIP;    // Current IP
extern String g_wifiSSID;  // Current SSID

/**
 * @brief GUI module for web-based control interface
 * 
 * This module provides a web-based user interface for controlling
 * and monitoring the IoT environmental monitoring system.
 */

/**
 * @brief Global WebServer instance
 */
extern WebServer server;

/**
 * @brief Initialize the GUI in AP mode
 * 
 * This function initializes the WiFi in AP mode, sets up the web server,
 * and registers all the necessary HTTP request handlers.
 */
void initGUI_AP();

/**
 * @brief Initialize the GUI in Station mode
 * 
 * This function initializes the WiFi in Station mode, 
 * in case there's an available WiFi source with known credentials
 * if not it falls back to initGUI_AP();
 */
bool initGUI_Station();

/**
 * @brief FreeRTOS task function for handling web requests
 * 
 * This task processes incoming web requests and serves the GUI.
 * 
 * @param pvParameters Task parameters (unused)
 */
void guiTask(void *pvParameters);

#endif // GUI_H
