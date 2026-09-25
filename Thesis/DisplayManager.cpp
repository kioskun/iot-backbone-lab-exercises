#include "DisplayManager.h"
#include "LoRaWAN.h"       // for connectionStatus
#include "SensorManager.h" // to read sensor display strings
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GUI.h"

/**
 * @brief OLED display pin definitions
 */
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/**
 * @brief OLED display instance
 */
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

/**
 * @brief Mutex for I2C bus access
 */
SemaphoreHandle_t i2cMutex = nullptr;

/**
 * @brief Internal function to update the display content
 * 
 * This function updates the display with current system status
 * and active sensor readings.
 */
static void internalUpdateDisplay() {
  // Clear display buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int y = 0;

  // 1) Show LoRa status
  display.setCursor(0, y);
  display.println(connectionStatus);
  y += 10;

  // 2) Show Wi-Fi info
  display.setCursor(0, y);
  display.println("WiFi: " + g_wifiMode);
  y += 10;

  display.setCursor(0, y);
  display.println("IP: " + g_wifiIP);
  y += 10;

  if (g_wifiMode == "Station") {
    display.setCursor(0, y);
    String shortSSID = g_wifiSSID.substring(0, 12);
    display.println("SSID: " + shortSSID);
    y += 10;
  }

  // 2) Show active sensors
  bool anyEnabled = false;
  for (auto s : sensorManager.sensors) {
    if (s->active) {
      anyEnabled = true;
      display.setCursor(0, y);
      display.println(s->getDisplayString()); 
      // e.g., "DHT: T=25.0C H=60.0%"
      y += 10;
      if (y >= (SCREEN_HEIGHT - 10)) break; // no more space
    }
  }
  
  // Show message if no sensors are enabled
  if (!anyEnabled) {
    display.setCursor(0, y);
    display.println("No sensor enabled");
  }
  
  // Update the display
  display.display();
}

/**
 * @brief Initialize the display manager
 * 
 * This function initializes the OLED display and I2C communication.
 */
void initDisplayManager() {
  // Create the mutex for I2C access
  i2cMutex = xSemaphoreCreateMutex();

  // Reset OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // Initialize I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println("SSD1306 allocation failed");
  }

  // Quick test message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Display Init");
  display.display();
}

/**
 * @brief FreeRTOS task function for updating the display
 * 
 * This task periodically updates the OLED display with
 * current system status and sensor readings.
 * 
 * @param pvParameters Task parameters (unused)
 */
void displayTask(void *pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    if (millis() < 5000) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }
    // Protect I2C access with mutex
    xSemaphoreTake(i2cMutex, portMAX_DELAY);
    
    // Update display content
    internalUpdateDisplay();
    
    // Release mutex
    xSemaphoreGive(i2cMutex);

    // Wait before next update
    vTaskDelay(pdMS_TO_TICKS(1000)); // update every second
  }
}
