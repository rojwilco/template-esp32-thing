// Include mocks BEFORE the sketch so angle-bracket includes resolve to mocks.
#include "mocks/Arduino.h"
#include "mocks/WiFi.h"
#include "mocks/Preferences.h"
#include "mocks/WebServer.h"
#include "mocks/Update.h"
#include "mocks/DNSServer.h"

// Compile the sketch as a host translation unit.
#include "../template-esp32-thing.ino"
