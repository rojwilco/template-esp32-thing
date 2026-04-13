#pragma once
#include <stdint.h>
#include <string.h>
#include <string>
#include <cstdio>
#include <algorithm>

// ---- String type (standalone, does NOT inherit std::string) ----
class String {
public:
    std::string _s;

    String() = default;
    String(const char* s) : _s(s ? s : "") {}
    String(const std::string& s) : _s(s) {}
    String(int v)           : _s(std::to_string(v)) {}
    String(unsigned int v)  : _s(std::to_string(v)) {}
    String(long v)          : _s(std::to_string(v)) {}
    String(unsigned long v) : _s(std::to_string(v)) {}
    String(float v,  int d=2) { char b[32]; snprintf(b,sizeof(b),"%.*f",d,(double)v); _s=b; }
    String(double v, int d=2) { char b[32]; snprintf(b,sizeof(b),"%.*f",d,v); _s=b; }

    // Explicit conversion so ArduinoJson / templates don't silently convert
    explicit operator std::string() const { return _s; }

    const char* c_str()  const { return _s.c_str(); }
    size_t length()      const { return _s.size(); }
    size_t size()        const { return _s.size(); }
    bool   isEmpty()     const { return _s.empty(); }

    String substring(size_t from, size_t to = std::string::npos) const {
        return String(_s.substr(from, to == std::string::npos ? std::string::npos : to - from));
    }
    void toCharArray(char* buf, size_t len) const {
        strncpy(buf, _s.c_str(), len);
        if(len) buf[len-1] = '\0';
    }
    int   toInt()   const { try{ return std::stoi(_s); } catch(...){ return 0; } }
    float toFloat() const { try{ return std::stof(_s); } catch(...){ return 0; } }

    bool startsWith(const char* p) const { return _s.rfind(p, 0) == 0; }

    void replace(const char* from, const char* to) {
        std::string f(from), t(to);
        size_t pos = 0;
        while((pos = _s.find(f, pos)) != std::string::npos) {
            _s.replace(pos, f.size(), t);
            pos += t.size();
        }
    }
    void replace(const String& from, const String& to) {
        replace(from.c_str(), to.c_str());
    }

    bool concat(const String& s) { _s += s._s; return true; }
    bool concat(const char* s)   { _s += s; return true; }

    String& operator+=(const String& o) { _s += o._s; return *this; }
    String& operator+=(const char* s)   { _s += s; return *this; }
    String  operator+(const String& o) const { return String(_s + o._s); }
    String  operator+(const char* s)   const { return String(_s + s); }
    bool operator==(const String& o) const { return _s == o._s; }
    bool operator==(const char* s)   const { return _s == s; }
    bool operator!=(const String& o) const { return !(*this == o); }
    bool operator< (const String& o) const { return _s < o._s; }
    char operator[](size_t i) const { return _s[i]; }
};

inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs) + rhs._s);
}

// ---- Constants ----
#define HIGH 1
#define LOW  0
#define INPUT  0
#define OUTPUT 1
#define WIFI_STA 1
#define WIFI_AP  2

// ---- Timing ----
extern unsigned long g_mock_millis;
inline unsigned long millis() { return g_mock_millis; }
inline void delay(unsigned long) {}

// ---- GPIO ----
inline void pinMode(uint8_t, uint8_t) {}
extern int g_mock_pin_value[40];
extern int g_mock_pin_mode[40];
inline void digitalWrite(uint8_t pin, uint8_t val) {
    if(pin < 40) g_mock_pin_value[pin] = val;
}
inline int digitalRead(uint8_t pin) {
    return (pin < 40) ? g_mock_pin_value[pin] : 0;
}

// ---- Serial ----
struct SerialClass {
    void begin(int) {}
    template<typename... A> void print(A&&...) {}
    template<typename... A> void println(A&&...) {}
    template<typename... A> int  printf(const char*, A&&...) { return 0; }
};
extern SerialClass Serial;

// ---- ESP ----
struct ESPClass {
    void restart() {}
};
extern ESPClass ESP;

// ---- Math ----
#ifndef constrain
#define constrain(x, lo, hi) ((x)<(lo)?(lo):((x)>(hi)?(hi):(x)))
#endif

// ---- PROGMEM no-op ----
#ifndef PROGMEM
#define PROGMEM
#endif
