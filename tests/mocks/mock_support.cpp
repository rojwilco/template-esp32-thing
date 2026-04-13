// Definitions for all mock global state.
// Compiled as a separate translation unit (not #included).
#include "Arduino.h"
#include "WiFi.h"
#include "Preferences.h"
#include "WebServer.h"
#include "Update.h"
#include <map>
#include <string>
#include <vector>

// Arduino
unsigned long g_mock_millis = 0;
int g_mock_pin_value[40] = {};
int g_mock_pin_mode[40]  = {};
SerialClass Serial;
ESPClass ESP;

// WiFi
int g_mock_wifi_status = WL_CONNECTED;
std::string g_mock_wifi_hostname;
std::vector<MockNetwork> g_mock_scan_results;
WiFiClass WiFi;

// Preferences
std::map<std::string, std::string> g_mock_prefs_store;

// WebServer
std::map<std::string,std::string> g_mock_server_args;
int    g_mock_last_send_code = 0;
String g_mock_last_send_body;
String g_mock_last_redirect;
HTTPUpload g_mock_upload;

// Update
bool   g_mock_update_begin_ok   = true;
bool   g_mock_update_write_fail = false;
bool   g_mock_update_end_ok     = true;
bool   g_mock_update_has_error  = false;
const char* g_mock_update_error_str = "";
size_t g_mock_update_written    = 0;
UpdateClass Update;

// ---- Reset all mock state between tests ----
void resetMocks() {
    g_mock_millis = 0;
    for(int i=0;i<40;i++){ g_mock_pin_value[i]=0; g_mock_pin_mode[i]=0; }

    g_mock_wifi_status = WL_CONNECTED;
    g_mock_wifi_hostname.clear();
    g_mock_scan_results.clear();

    g_mock_prefs_store.clear();

    g_mock_server_args.clear();
    g_mock_last_send_code = 0;
    g_mock_last_send_body = String();
    g_mock_last_redirect  = String();

    g_mock_update_begin_ok   = true;
    g_mock_update_write_fail = false;
    g_mock_update_end_ok     = true;
    g_mock_update_has_error  = false;
    g_mock_update_error_str  = "";
    g_mock_update_written    = 0;
}
