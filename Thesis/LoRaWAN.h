#ifndef LORAWAN_H
#define LORAWAN_H

#include <Arduino.h>
#include <lmic.h>

/**
 * @brief LoRaWAN communication module
 * 
 * This module handles LoRaWAN communication with The Things Network (TTN).
 * It manages OTAA join process and periodic data transmission.
 */

/**
 * @brief A string for the display code to show LoRa status
 * 
 * This variable is updated with the current connection status
 * and can be displayed on the OLED screen.
 */
extern String connectionStatus;

/**
 * @brief A string for storing the time (in seconds) of the last transmission.
 */
extern String lastTx;

/**
 * @brief Initialize LMIC and start OTAA join
 * 
 * This function initializes the LoRaWAN stack and starts
 * the OTAA (Over-The-Air Activation) join process.
 */
void initLoRaWAN();

/**
 * @brief Send sensor data over LoRaWAN
 * 
 * This function is called periodically to send sensor data
 * to The Things Network. It's typically scheduled by the LMIC
 * event handler after a successful join.
 * 
 * @param j Pointer to the LMIC job structure
 */
void do_send(osjob_t* j);

/**
 * @brief LMIC event handler
 * 
 * This function handles LoRaWAN events such as join requests,
 * join completion, and transmission completion.
 * 
 * @param ev Event type
 */
void onEvent(ev_t ev);

#endif // LORAWAN_H
