/*
 * template-esp32-thing
 * Generic ESP32 firmware template: WiFi, NVS config, WebServer, OTA, GPIO toggle.
 * Replace the GPIO toggle placeholder with your actual peripheral logic.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Update.h>
#include <DNSServer.h>
#include "version.h"
#include "config_html.h"

// ---------------------------------------------------------------------------
// Configuration (persisted to NVS under namespace "esp32thing")
// ---------------------------------------------------------------------------
char     cfg_wifi_ssid[64]          = "";
char     cfg_wifi_pass[64]          = "";
uint8_t  cfg_gpio_pin               = 2;     // built-in LED on most ESP32 boards
uint32_t cfg_toggle_interval_ms     = 1000;  // ms between GPIO toggles
uint16_t cfg_poll_interval_min      = 1;     // minutes between main task runs

// ---------------------------------------------------------------------------
// Runtime flags
// ---------------------------------------------------------------------------
bool g_pendingConnect = false;  // set when WiFi credentials change
bool g_forceRerun     = false;  // set when task-relevant config changes

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------
static WebServer   server(80);
static Preferences prefs;
static DNSServer   dnsServer;
static bool        g_apMode       = false;
static bool        g_gpioState    = false;
static uint32_t    g_lastTaskMs   = 0;

// ---------------------------------------------------------------------------
// NVS helpers
// ---------------------------------------------------------------------------
void loadConfig() {
    prefs.begin("esp32thing", true);
    prefs.getString("wifi_ssid", cfg_wifi_ssid, sizeof(cfg_wifi_ssid));
    prefs.getString("wifi_pass", cfg_wifi_pass, sizeof(cfg_wifi_pass));
    cfg_gpio_pin           = prefs.getUChar("gpio_pin",     cfg_gpio_pin);
    cfg_toggle_interval_ms = prefs.getULong("toggle_ms",    cfg_toggle_interval_ms);
    cfg_poll_interval_min  = prefs.getUShort("poll_min",    cfg_poll_interval_min);
    prefs.end();

    // Clamp values to valid ranges
    if (cfg_gpio_pin > 39)                cfg_gpio_pin = 2;
    if (cfg_toggle_interval_ms < 100)     cfg_toggle_interval_ms = 100;
    if (cfg_poll_interval_min < 1)        cfg_poll_interval_min = 1;
    if (cfg_poll_interval_min > 1440)     cfg_poll_interval_min = 1440;
}

void saveConfig() {
    prefs.begin("esp32thing", false);
    prefs.putString("wifi_ssid", cfg_wifi_ssid);
    prefs.putString("wifi_pass", cfg_wifi_pass);
    prefs.putUChar("gpio_pin",   cfg_gpio_pin);
    prefs.putULong("toggle_ms",  cfg_toggle_interval_ms);
    prefs.putUShort("poll_min",  cfg_poll_interval_min);
    prefs.end();
}

// ---------------------------------------------------------------------------
// Hostname: "esp32-thing-XXXXXX" from last 3 MAC bytes
// ---------------------------------------------------------------------------
void applyHostname() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char hostname[32];
    snprintf(hostname, sizeof(hostname), "esp32-thing-%02x%02x%02x",
             mac[3], mac[4], mac[5]);
    WiFi.setHostname(hostname);
}

// ---------------------------------------------------------------------------
// AP mode: soft AP + captive-portal DNS
// ---------------------------------------------------------------------------
void startAPMode() {
    g_apMode = true;
    WiFi.mode(WIFI_AP);
    applyHostname();
    WiFi.softAP("ESP32-Thing-Setup");
    dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.printf("[WiFi] AP mode: SSID=ESP32-Thing-Setup, IP=%s\n",
                  WiFi.softAPIP().toString().c_str());
}

// ---------------------------------------------------------------------------
// Periodic task: GPIO toggle (replace with your peripheral logic)
// ---------------------------------------------------------------------------
void runTask() {
    g_forceRerun = false;

    g_gpioState = !g_gpioState;
    digitalWrite(cfg_gpio_pin, g_gpioState ? HIGH : LOW);
    Serial.printf("[Task] GPIO %d -> %s\n", cfg_gpio_pin, g_gpioState ? "HIGH" : "LOW");
}

// ---------------------------------------------------------------------------
// Web handlers
// ---------------------------------------------------------------------------
void handleRoot() {
    String page = String(CONFIG_HTML);
    page.replace("{{VERSION}}", FIRMWARE_VERSION);
    page.replace("{{WIFI_SSID}}", cfg_wifi_ssid);
    // Never pre-fill password
    page.replace("{{GPIO_PIN}}",      String(cfg_gpio_pin));
    page.replace("{{TOGGLE_MS}}",     String(cfg_toggle_interval_ms));
    page.replace("{{POLL_MIN}}",      String(cfg_poll_interval_min));
    page.replace("{{AP_MODE}}", g_apMode ? "true" : "false");

    server.send(200, "text/html", page);
}

void handleSave() {
    bool wifiChanged = false;
    bool taskChanged = false;

    if (server.hasArg("wifi_ssid")) {
        String v = server.arg("wifi_ssid");
        if (v.length() >= sizeof(cfg_wifi_ssid))
            v = v.substring(0, sizeof(cfg_wifi_ssid) - 1);
        if (strcmp(v.c_str(), cfg_wifi_ssid) != 0) wifiChanged = true;
        strncpy(cfg_wifi_ssid, v.c_str(), sizeof(cfg_wifi_ssid));
        cfg_wifi_ssid[sizeof(cfg_wifi_ssid) - 1] = '\0';
    }
    if (server.hasArg("wifi_pass")) {
        String v = server.arg("wifi_pass");
        if (v.length() >= sizeof(cfg_wifi_pass))
            v = v.substring(0, sizeof(cfg_wifi_pass) - 1);
        if (strcmp(v.c_str(), cfg_wifi_pass) != 0) wifiChanged = true;
        strncpy(cfg_wifi_pass, v.c_str(), sizeof(cfg_wifi_pass));
        cfg_wifi_pass[sizeof(cfg_wifi_pass) - 1] = '\0';
    }
    if (server.hasArg("gpio_pin")) {
        uint8_t v = (uint8_t)server.arg("gpio_pin").toInt();
        if (v > 39) v = 2;
        if (v != cfg_gpio_pin) taskChanged = true;
        cfg_gpio_pin = v;
    }
    if (server.hasArg("toggle_ms")) {
        uint32_t v = (uint32_t)server.arg("toggle_ms").toInt();
        if (v < 100) v = 100;
        if (v != cfg_toggle_interval_ms) taskChanged = true;
        cfg_toggle_interval_ms = v;
    }
    if (server.hasArg("poll_min")) {
        uint16_t v = (uint16_t)server.arg("poll_min").toInt();
        if (v < 1) v = 1;
        if (v > 1440) v = 1440;
        cfg_poll_interval_min = v;
    }

    saveConfig();

    if (wifiChanged) g_pendingConnect = true;
    if (taskChanged) g_forceRerun     = true;

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleScan() {
    int n = WiFi.scanNetworks();
    // Build sorted, de-duped list
    struct AP { String ssid; int32_t rssi; };
    std::vector<AP> aps;
    for (int i = 0; i < n; i++) {
        String s = WiFi.SSID(i);
        bool dup = false;
        for (auto& a : aps) { if (a.ssid == s) { dup = true; break; } }
        if (!dup) aps.push_back({s, WiFi.RSSI(i)});
    }
    std::sort(aps.begin(), aps.end(),
              [](const AP& a, const AP& b){ return a.rssi > b.rssi; });

    String json = "[";
    for (size_t i = 0; i < aps.size(); i++) {
        if (i) json += ",";
        json += "{\"ssid\":\"" + aps[i].ssid + "\",\"rssi\":" + String(aps[i].rssi) + "}";
    }
    json += "]";
    server.send(200, "application/json", json);
}

void handleOtaUpdate() {
    server.sendHeader("Connection", "close");
    bool ok = !Update.hasError();
    server.send(ok ? 200 : 500, "text/plain", ok ? "OK" : "FAIL");
    if (ok) {
        delay(500);
        ESP.restart();
    }
}

void handleOtaUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA] Start: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("[OTA] Success: %u bytes\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.printf("\n[Boot] %s (%s)\n", FIRMWARE_VERSION, FIRMWARE_BUILD_TIMESTAMP);

    loadConfig();

    pinMode(cfg_gpio_pin, OUTPUT);
    digitalWrite(cfg_gpio_pin, LOW);

    // WiFi: try station mode; fall back to AP
    if (strlen(cfg_wifi_ssid) > 0) {
        WiFi.mode(WIFI_STA);
        applyHostname();
        WiFi.begin(cfg_wifi_ssid, cfg_wifi_pass);
        Serial.printf("[WiFi] Connecting to %s", cfg_wifi_ssid);
        uint32_t t = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[WiFi] Connected: %s\n", WiFi.localIP().toString().c_str());
            g_apMode = false;
        } else {
            Serial.println("[WiFi] Failed, starting AP");
            startAPMode();
        }
    } else {
        startAPMode();
    }

    server.on("/",          HTTP_GET,  handleRoot);
    server.on("/save",      HTTP_POST, handleSave);
    server.on("/scan",      HTTP_GET,  handleScan);
    server.on("/update",    HTTP_POST, handleOtaUpdate, handleOtaUpload);
    server.onNotFound([]() {
        server.sendHeader("Location", "/");
        server.send(302);
    });
    server.begin();
    Serial.println("[HTTP] Server started");
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {
    if (g_apMode) dnsServer.processNextRequest();
    server.handleClient();

    // Reconnect when credentials changed
    if (g_pendingConnect) {
        g_pendingConnect = false;
        WiFi.disconnect(true);
        delay(200);
        WiFi.mode(WIFI_STA);
        applyHostname();
        WiFi.begin(cfg_wifi_ssid, cfg_wifi_pass);
        uint32_t t = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
            delay(500);
        }
        if (WiFi.status() == WL_CONNECTED) {
            g_apMode = false;
        }
    }

    uint32_t now = millis();
    uint32_t intervalMs = (uint32_t)cfg_poll_interval_min * 60000UL;

    if (g_forceRerun || (now - g_lastTaskMs >= intervalMs)) {
        g_lastTaskMs = now;
        runTask();
    }
}
