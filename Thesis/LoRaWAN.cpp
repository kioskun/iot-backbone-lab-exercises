#include "LoRaWAN.h"
#include "SensorManager.h"  // for sensorManager.getSensorDataJSON()
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>


/**
 * @brief Current LoRaWAN connection status
 */
String connectionStatus = "Initializing...";

/**
 * @brief Job structure for scheduling transmissions
 */
static osjob_t sendjob;

/**
 * @brief Transmission interval in seconds
 */
static const unsigned TX_INTERVAL = 60;

/**
 * @brief TTN Application EUI (LSB format)
 * 
 * This should be replaced with your TTN Application EUI
 */
static const u1_t PROGMEM APPEUI[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/**
 * @brief Callback to get Application EUI
 */
void os_getArtEui(u1_t* buf) { 
    memcpy_P(buf, APPEUI, 8); 
}

/**
 * @brief TTN Device EUI (LSB format)
 * 
 * This should be replaced with your TTN Device EUI
 */
static const u1_t PROGMEM DEVEUI[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/**
 * @brief Callback to get Device EUI
 */
void os_getDevEui(u1_t* buf) { 
    memcpy_P(buf, DEVEUI, 8); 
}

/**
 * @brief TTN Application Key (MSB format)
 * 
 * This should be replaced with your TTN Application Key
 */
static const u1_t PROGMEM APPKEY[16] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

/**
 * @brief Callback to get Application Key
 */
void os_getDevKey(u1_t* buf) { 
    memcpy_P(buf, APPKEY, 16); 
}

/**
 * @brief TTGO LoRa32 SX1276 Pin Mapping
 * 
 * This defines the pin connections between the ESP32 and the SX1276 radio module
 */
const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = {26, 33, 32},
};

/**
 * @brief Initialize LMIC and start OTAA join
 * 
 * This function initializes the LoRaWAN stack and starts
 * the OTAA (Over-The-Air Activation) join process.
 */

String lastTx = "--:--:--";

void initLoRaWAN() {
  Serial.println("Initializing LMIC...");
  
  // Initialize LMIC
  os_init();
  
  // Reset LMIC state
  LMIC_reset();
  
  // Set clock error to allow larger timing windows for reception
  LMIC_setClockError(MAX_CLOCK_ERROR * 4 / 100);

  // Force an OTAA join
  LMIC_startJoining();

  connectionStatus = "LoRa init done";
}

/**
 * @brief Send sensor data over LoRaWAN
 * 
 * This function is called periodically to send sensor data
 * to The Things Network. It's typically scheduled by the LMIC
 * event handler after a successful join.
 * 
 * @param j Pointer to the LMIC job structure
 */
void do_send(osjob_t* j) {
  // If we're mid-transmission, skip
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println("OP_TXRXPEND, not sending");
    connectionStatus = "TX busy";
    return;
  }

  // Gather active sensor data in JSON
  String payloadStr = sensorManager.getSensorDataJSON();
  if (payloadStr == "{}") {
    payloadStr = "{\"dummy\":1}";
  }
  uint8_t len = payloadStr.length();

  // Queue the packet for transmission
  LMIC_setTxData2(1, (uint8_t*)payloadStr.c_str(), len, 0);
  
  Serial.print("Packet queued: ");
  Serial.println(payloadStr);
  connectionStatus = "TX Sending";
}

/**
 * @brief LMIC event handler
 * 
 * This function handles LoRaWAN events such as join requests,
 * join completion, and transmission completion.
 * 
 * @param ev Event type
 */
void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  
  switch(ev) {
    case EV_JOINING:
      Serial.println("EV_JOINING");
      connectionStatus = "Joining TTN...";
      break;

    case EV_JOINED:
      Serial.println("EV_JOINED");
      connectionStatus = "Joined TTN!";
      LMIC_setLinkCheckMode(0);
      // Immediately schedule our first send
      do_send(&sendjob);
      break;

    case EV_JOIN_FAILED:
      Serial.println("EV_JOIN_FAILED");
      connectionStatus = "Join Failed";
      break;

    case EV_TXSTART:
      Serial.println("EV_TXSTART");
      connectionStatus = "TX Start";
      break;

    case EV_TXCOMPLETE:
      Serial.println("EV_TXCOMPLETE");
      connectionStatus = "TX Complete";
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println("Received ack");
      }
      if (LMIC.dataLen) {
        Serial.print("Received ");
        Serial.print(LMIC.dataLen);
        Serial.println(" bytes of payload");
      }
      // Update last transmission time (for example, using uptime in seconds)
      lastTx = String(millis() / 1000);
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;

    default:
      Serial.print("Unknown event: ");
      Serial.println((unsigned) ev);
      connectionStatus = "Unknown event";
      break;
  }
}
