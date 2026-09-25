  #include "GUI.h"
#include "SensorManager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "LoRaWAN.h"
#include <ArduinoJson.h> // For parsing/storing JSON credentials

String g_wifiMode  = "";
String g_wifiIP    = "";
String g_wifiSSID  = "";

/**
 * @brief Our fallback AP credentials
 */
static const char* apSSID = "REPLACE_AP_SSID";
static const char* apPassword = "REPLACE_AP_PASSWORD";

/**
 * @brief Global WebServer on port 80
 */
WebServer server(80);

// --- Default fallback Wi-Fi credentials (Station mode will try these if no saved creds or saved creds fail)
static const char* DEFAULT_WIFI_SSID     = "REPLACE_WIFI_SSID";
static const char* DEFAULT_WIFI_PASSWORD = "REPLACE_WIFI_PASSWORD";


// Forward declaration so we can call it before its definition
static void setupCommonRoutes();


// Serve ONLY plain files (no .gz), to rule out encoding/stream quirks
static void sendFilePlainOnly(const char* fsPath, const char* contentType, bool cache = true) {
  String p = fsPath;
  if (p.length() && p[0] != '/') p = "/" + p;

  if (!LittleFS.exists(p)) {
    Serial.printf("[404-plain] %s\n", p.c_str());
    server.send(404, "text/plain", "Not found: " + p);
    return;
  }
  File f = LittleFS.open(p, "r");
  if (!f) { server.send(500, "text/plain", "Open fail: " + p); return; }

  if (cache) server.sendHeader("Cache-Control", "public, max-age=86400");
  Serial.printf("[HTTP-plain] %s <- %s\n", contentType, p.c_str());
  server.streamFile(f, contentType);
  f.close();
}

static bool ensureFS() {
  static bool mounted = false;
  if (!mounted) {
    mounted = LittleFS.begin(true); // true = auto-format if needed
    if (!mounted) Serial.println("[FS] LittleFS mount failed in ensureFS()");
  }
  return mounted;
}

/**
 * @brief Attempt to load Wi-Fi credentials from LittleFS
 * @param ssidOut String to store loaded SSID
 * @param passOut String to store loaded password
 * @return true if loaded successfully, false otherwise
 */
bool loadWifiCredentials(String &ssidOut, String &passOut) {

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return false;
  }
  if (!LittleFS.exists("/wifi_credentials.json")) {
    Serial.println("No wifi_credentials.json found");
    return false;
  }
  File f = LittleFS.open("/wifi_credentials.json", "r");
  if (!f) {
    Serial.println("Failed to open wifi_credentials.json");
    return false;
  }
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    Serial.print("Failed to parse wifi_credentials.json: ");
    Serial.println(err.c_str());
    return false;
  }
  ssidOut   = doc["ssid"].as<String>();
  passOut   = doc["password"].as<String>();
  return true;
}

/**
 * @brief Save new Wi-Fi credentials to LittleFS
 */
bool saveWifiCredentials(const String &newSSID, const String &newPassword) {
  StaticJsonDocument<256> doc;
  doc["ssid"]     = newSSID;
  doc["password"] = newPassword;

  File f = LittleFS.open("/wifi_credentials.json", "w");
  if (!f) {
    Serial.println("Failed to open wifi_credentials.json for writing");
    return false;
  }
  serializeJson(doc, f);
  f.close();
  Serial.println("Saved new Wi-Fi credentials to /wifi_credentials.json");
  return true;
}

/**
 * @brief Connect in station mode using stored credentials
 * @return true if connected, false otherwise
 */
bool connectStationMode(const String &ssid, const String &pass) {
  Serial.println("Trying station mode with SSID: " + ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long startAttempt = millis();
  const unsigned long timeout = 10000; // 10s

  while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < timeout) {
    Serial.print(".");
    delay(250);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected in station mode! IP: " + WiFi.localIP().toString());
    return true;
  } else {
    Serial.println("Station connect failed/timed out");
    return false;
  }
}

bool initGUI_Station() {
  String storedSSID, storedPass;
  bool loaded = loadWifiCredentials(storedSSID, storedPass);
  bool connected = false;
  bool usedDefault = false;

  // 1) Try stored credentials
  if (loaded && !storedSSID.isEmpty()) {
    Serial.printf("Trying stored Wi-Fi: %s ...\n", storedSSID.c_str());
    connected = connectStationMode(storedSSID, storedPass);
  }

  // 2) Try default if not connected yet
  if (!connected && DEFAULT_WIFI_SSID && DEFAULT_WIFI_SSID[0] != '\0') {
    Serial.printf("Trying default Wi-Fi: %s ...\n", DEFAULT_WIFI_SSID);
    connected = connectStationMode(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    usedDefault = connected;
  }

  if (!connected) {
    Serial.println("No Wi-Fi network reachable, falling back to AP.");
    return false;
  }

  // Save the default credentials if none were stored and the default network worked
  if (usedDefault && !loaded) {
    saveWifiCredentials(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
  }

  g_wifiMode = "Station";
  g_wifiIP   = WiFi.localIP().toString();
  g_wifiSSID = WiFi.SSID();

  ensureFS();
  setupCommonRoutes();
  server.begin();

  Serial.println("Web server started in station mode at: " + WiFi.localIP().toString());
  return true;
}




/**
 * @brief Setup the web server routes (common to both station or AP)
 */
static void setupCommonRoutes() {

  server.on("/", HTTP_GET, [](){           sendFilePlainOnly("/index.html", "text/html", false); });
  server.on("/index.html", HTTP_GET, [](){ sendFilePlainOnly("/index.html", "text/html", false); });
  server.on("/style.css", HTTP_GET, [](){  sendFilePlainOnly("/style.css", "text/css"); });
  server.on("/script.js", HTTP_GET, [](){  sendFilePlainOnly("/script.js", "text/javascript"); });
  server.on("/chart.umd.min.js", HTTP_GET, [](){
    sendFilePlainOnly("/chart.umd.min.js", "text/javascript");
  });
  server.on("/control_panel.partial.html", HTTP_GET, [](){
    sendFilePlainOnly("/control_panel.partial.html", "text/html", false);
  });
  server.on("/dht_setup.partial.html", HTTP_GET, [](){
    sendFilePlainOnly("/dht_setup.partial.html", "text/html", false);
  });
  server.on("/photo_setup.partial.html", HTTP_GET, [](){
    sendFilePlainOnly("/photo_setup.partial.html", "text/html", false);
  });
  server.on("/wifi_setup.partial.html", HTTP_GET, [](){
    sendFilePlainOnly("/wifi_setup.partial.html", "text/html", false);
  });



  // Sensor command
  server.on("/sensor", HTTP_GET, [](){
    if (server.hasArg("sensor") && server.hasArg("action")) {
      String sname = server.arg("sensor");
      String act   = server.arg("action");
      
      if (act.equalsIgnoreCase("enable")) {
        sensorManager.enableSensor(sname);
        server.send(200, "text/plain", "Enabled sensor: " + sname);
      } else if (act.equalsIgnoreCase("disable")) {
        sensorManager.disableSensor(sname);
        server.send(200, "text/plain", "Disabled sensor: " + sname);
      } else {
        server.send(400, "text/plain", "Unknown action");
      }
    } else {
      server.send(400, "text/plain", "Missing sensor/action params");
    }
  });

  server.on("/getWifiCreds", HTTP_GET, [](){
    // Load the stored credentials and return them as JSON
    String ssid, pass;
    bool ok = loadWifiCredentials(ssid, pass);
    if (!ok) {
      // No file found or parse error
      server.send(200, "application/json", "{\"ssid\":\"\",\"password\":\"\"}");
      return;
    }
    // Return them in JSON
    // The password is returned in plain text (see KNOWN_LIMITATIONS.md)
    String json = "{\"ssid\":\"" + ssid + "\",\"password\":\"" + pass + "\"}";
    server.send(200, "application/json", json);
  });

  server.on("/deployWifi", HTTP_POST, [](){
    if (!server.hasArg("ssid") || !server.hasArg("password")) {
      server.send(400, "text/plain", "Missing ssid/password");
      return;
    }
    String newSSID = server.arg("ssid");
    String newPass = server.arg("password");

    if (saveWifiCredentials(newSSID, newPass)) {
      server.send(200, "text/plain", "Credentials saved. Rebooting...");
      delay(500);
      ESP.restart(); 
    } else {
      server.send(500, "text/plain", "Failed to save credentials.");
    }
  });

  // Status
  server.on("/status", HTTP_GET, [](){
    // Increase the document size when more sensors or fields are added
    StaticJsonDocument<512> doc;

    // 1) toggles (enabled/disabled)
    for (auto s : sensorManager.sensors) {
      doc[s->getName()] = s->active; // e.g., "DHT": true
    }

    // 2) system info
    doc["uptime"] = millis() / 1000;
    doc["lastTx"] = lastTx.toInt(); // stored as "123" string in code

    // 3) sensor readings (merge JSON string returned by SensorManager)
    String sensorJson = sensorManager.getSensorDataJSON(); // e.g., {"DHT_Temp":..,"DHT_Hum":..}
    if (sensorJson.length() > 2) {
      StaticJsonDocument<256> sensorDoc;
      DeserializationError err = deserializeJson(sensorDoc, sensorJson);
      if (!err) {
        for (JsonPair kv : sensorDoc.as<JsonObject>()) {
          doc[kv.key()] = kv.value(); // merge fields into main doc
        }
      }
    }

    // 4) send compact JSON + cache short
    String out;
    serializeJson(doc, out);
    server.sendHeader("Cache-Control", "no-store"); // always fresh for live data
    server.send(200, "application/json", out);
  });

  // ---------- Wi-Fi configuration route ----------
  // Example: /configureWifi?ssid=<SSID>&password=<PASSWORD>
  server.on("/configureWifi", HTTP_GET, [](){
    if (!server.hasArg("ssid") || !server.hasArg("password")) {
      server.send(400, "text/plain", "Missing ssid/password");
      return;
    }
    String newSSID = server.arg("ssid");
    String newPass = server.arg("password");

    if (saveWifiCredentials(newSSID, newPass)) {
      server.send(200, "text/plain", 
        "Credentials saved. Reboot or power cycle to use new WiFi.");
    } else {
      server.send(500, "text/plain", "Failed to save credentials.");
    }
  });
}

/**
 * @brief Attempt to run in station mode if we have credentials. 
 * @return true if connected in station mode, false otherwise
 */

/**
 * @brief Start the fallback access point (AP) mode
 */
#include <esp_wifi.h>

void initGUI_AP() {
  ensureFS();

  // Put Wi-Fi into AP mode
  WiFi.mode(WIFI_AP);

  // Fixed 2.4 GHz channel (1, 6 or 11). Another of these channels
  // can be used if the band is congested.
  const int AP_CHANNEL = 6;
  const int AP_MAX_CONN = 2;   // keep small to reduce contention
  const bool AP_HIDDEN = false;

  // Start the AP with a fixed channel and a maximum number of clients
  WiFi.softAP(apSSID, apPassword, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONN);

  // Disable Wi-Fi power saving in AP mode for faster responses
  WiFi.setSleep(false);

  // Maximum ESP32 transmit power (within regulatory limits)
  //    Arduino-style API:
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  // Equivalent ESP-IDF call:
  // esp_wifi_set_max_tx_power(78); // 78 * 0.25 dBm = 19.5 dBm

  // Hostname of the AP (helps some operating systems)
  WiFi.softAPsetHostname("ESP32-IoT-Monitor");

  Serial.println("AP IP: " + WiFi.softAPIP().toString());

  g_wifiMode = "AP";
  g_wifiIP   = WiFi.softAPIP().toString();
  g_wifiSSID = apSSID;

  setupCommonRoutes();
  server.begin();
  Serial.println("Web server started in AP mode");
}


/**
 * @brief FreeRTOS task function for handling web requests
 * 
 * This task processes incoming web requests and serves the GUI.
 * 
 * @param pvParameters Task parameters (unused)
 */
void guiTask(void *pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
