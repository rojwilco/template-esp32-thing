#pragma once
#include "Arduino.h"

class DNSServer {
public:
    void start(uint16_t, const char*, const IPAddress&) {}
    void processNextRequest() {}
    void stop() {}
};
