#pragma once
#include "Arduino.h"
#include <vector>
#include <string>

struct IPAddress {
    IPAddress() {}
    String toString() const { return String("192.168.1.1"); }
};

// Status / mode constants
#define WL_CONNECTED    3
#define WL_DISCONNECTED 6

struct MockNetwork {
    std::string ssid;
    int rssi;
};

// Mock state (defined in mock_support.cpp)
extern int g_mock_wifi_status;
extern std::string g_mock_wifi_hostname;
extern std::vector<MockNetwork> g_mock_scan_results;

class WiFiClass {
public:
    void mode(int) {}
    void begin(const char*, const char*) {}
    int  status() { return g_mock_wifi_status; }
    void disconnect(bool) {}

    void macAddress(uint8_t* buf) {
        // Fixed test MAC: AA:BB:CC:11:22:33
        static const uint8_t mac[6] = {0xAA,0xBB,0xCC,0x11,0x22,0x33};
        for(int i=0;i<6;i++) buf[i]=mac[i];
    }
    String macAddress() { return String("AA:BB:CC:11:22:33"); }
    void setHostname(const char* h) { g_mock_wifi_hostname = h; }

    IPAddress localIP()  { return IPAddress(); }
    IPAddress softAPIP() { return IPAddress(); }
    bool softAP(const char*, const char* = "") { return true; }
    bool softAPdisconnect() { return true; }

    int    scanNetworks() { return (int)g_mock_scan_results.size(); }
    String SSID(int i) {
        if(i < (int)g_mock_scan_results.size())
            return String(g_mock_scan_results[i].ssid.c_str());
        return String("");
    }
    int32_t RSSI(int i) {
        if(i < (int)g_mock_scan_results.size()) return g_mock_scan_results[i].rssi;
        return 0;
    }
};
extern WiFiClass WiFi;
