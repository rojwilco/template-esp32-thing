#pragma once

// Mock headers — tests include this file to get all mock globals and sketch externs.
#include "mocks/Arduino.h"
#include "mocks/WiFi.h"
#include "mocks/Preferences.h"
#include "mocks/WebServer.h"
#include "mocks/Update.h"

// ---- Config variables (defined in sketch) ----
extern char     cfg_wifi_ssid[64];
extern char     cfg_wifi_pass[64];
extern uint8_t  cfg_gpio_pin;
extern uint32_t cfg_toggle_interval_ms;
extern uint16_t cfg_poll_interval_min;

// ---- Runtime flags (defined in sketch) ----
extern bool g_pendingConnect;
extern bool g_forceRerun;

// ---- Sketch functions under test ----
void loadConfig();
void saveConfig();
void applyHostname();
void runTask();
void handleRoot();
void handleSave();
void handleScan();
void handleOtaUpdate();
void handleOtaUpload();

// ---- Firmware version ----
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.1.0-dev"
#endif

// ---- Mock reset helper (defined in mock_support.cpp) ----
void resetMocks();
