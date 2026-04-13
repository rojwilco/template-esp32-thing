#pragma once
#include <string>
#include <map>
#include <cstring>
#include <type_traits>
#include "Arduino.h"

// In-memory NVS backing store (defined in mock_support.cpp)
extern std::map<std::string, std::string> g_mock_prefs_store;

class Preferences {
    std::string _ns;

    std::string key(const char* k) const { return _ns + "/" + k; }

    template<typename T>
    T get_or(const char* k, T def) const {
        auto it = g_mock_prefs_store.find(key(k));
        if(it == g_mock_prefs_store.end()) return def;
        if constexpr (std::is_same_v<T, float>)
            return (T)std::stof(it->second);
        else if constexpr (std::is_floating_point_v<T>)
            return (T)std::stod(it->second);
        else
            return (T)std::stoull(it->second);
    }

public:
    bool begin(const char* ns, bool = false) { _ns = ns; return true; }
    void end() {}

    // Getters
    uint8_t  getUChar (const char* k, uint8_t  d=0) { return get_or<uint8_t>(k,d); }
    uint16_t getUShort(const char* k, uint16_t d=0) { return get_or<uint16_t>(k,d); }
    uint32_t getUInt  (const char* k, uint32_t d=0) { return get_or<uint32_t>(k,d); }
    uint32_t getULong (const char* k, uint32_t d=0) { return get_or<uint32_t>(k,d); }
    float    getFloat (const char* k, float    d=0) { return get_or<float>(k,d); }

    size_t getString(const char* k, char* buf, size_t maxLen) {
        auto it = g_mock_prefs_store.find(key(k));
        if(it == g_mock_prefs_store.end()) { if(buf&&maxLen) buf[0]='\0'; return 0; }
        strncpy(buf, it->second.c_str(), maxLen);
        if(maxLen) buf[maxLen-1] = '\0';
        return it->second.size();
    }
    String getString(const char* k, String def="") {
        auto it = g_mock_prefs_store.find(key(k));
        if(it == g_mock_prefs_store.end()) return def;
        return String(it->second.c_str());
    }

    // Putters
    size_t putUChar (const char* k, uint8_t  v) { g_mock_prefs_store[key(k)]=std::to_string(v); return 1; }
    size_t putUShort(const char* k, uint16_t v) { g_mock_prefs_store[key(k)]=std::to_string(v); return 2; }
    size_t putUInt  (const char* k, uint32_t v) { g_mock_prefs_store[key(k)]=std::to_string(v); return 4; }
    size_t putULong (const char* k, uint32_t v) { g_mock_prefs_store[key(k)]=std::to_string(v); return 4; }
    size_t putFloat (const char* k, float    v) { g_mock_prefs_store[key(k)]=std::to_string(v); return 4; }
    size_t putString(const char* k, const char* v)  { g_mock_prefs_store[key(k)]=v; return strlen(v); }
    size_t putString(const char* k, const String& v){ g_mock_prefs_store[key(k)]=v.c_str(); return v.length(); }

    bool isKey(const char* k) { return g_mock_prefs_store.count(key(k))>0; }
};
