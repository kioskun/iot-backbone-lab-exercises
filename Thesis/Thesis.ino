#include <Arduino.h>
#include <MySensorLib.h>   // umbrella: SensorManager, DisplayManager, GUI, LoRaWAN
#include <LittleFS.h>

// MCCI LMIC forward (needed on some cores)
extern "C" { void os_runloop_once(void); }

// Optional FreeRTOS Task handles
TaskHandle_t sensorTaskHandle  = nullptr;
TaskHandle_t displayTaskHandle = nullptr;
TaskHandle_t guiTaskHandle     = nullptr;

static void tryStationModeOrFallback() {
  bool stationConnected = initGUI_Station();
  if (!stationConnected) {
    Serial.println("Falling back to AP mode…");
    initGUI_AP();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed; GUI will try again internally when needed.");
  }

  // Order: radio → display → sensors → GUI
  initLoRaWAN();         // LMIC init + start OTAA join (Core 0 sensitive)
  initDisplayManager();  // OLED init (Core 1 later)
  initSensorManager();   // DHT / Photo init

  tryStationModeOrFallback(); // starts server

  // ---------- TASK PINNING & PRIORITIES ----------
  // Core indices: 0 and 1 on ESP32
  // Keep Web + LMIC with the WiFi stack on Core 0
  // Move Sensors + Display to Core 1

  // Web server task: Core 0, priority 2 (slightly higher than default 1)
  xTaskCreatePinnedToCore(
    guiTask, "GUITask",
    4096,      // stack size in bytes
    nullptr,
    2,         // priority
    &guiTaskHandle,
    0          // Core 0
  );

  // Display task: Core 1, priority 1
  xTaskCreatePinnedToCore(
    displayTask, "DisplayTask",
    4096, nullptr, 1, &displayTaskHandle, 1
  );

  // Sensor task: Core 1, priority 1
  xTaskCreatePinnedToCore(
    sensorTask, "SensorTask",
    4096, nullptr, 1, &sensorTaskHandle, 1
  );

  Serial.println("Setup done.");
}

void loop() {
  // Keep LMIC alive for join/TX scheduling (Core 0)
  os_runloop_once();
  // Short pause between LMIC iterations
  delay(5);
}
