#pragma once
#include <stdint.h>
#include <cstddef>

#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

// Mock state (defined in mock_support.cpp)
extern bool   g_mock_update_begin_ok;
extern bool   g_mock_update_write_fail;
extern bool   g_mock_update_end_ok;
extern bool   g_mock_update_has_error;
extern const char* g_mock_update_error_str;
extern size_t g_mock_update_written;

class UpdateClass {
public:
    bool begin(size_t = UPDATE_SIZE_UNKNOWN) {
        g_mock_update_written = 0;
        return g_mock_update_begin_ok;
    }
    size_t write(uint8_t* buf, size_t len) {
        if(g_mock_update_write_fail) return 0;
        g_mock_update_written += len;
        return len;
    }
    bool end(bool = true)  { return g_mock_update_end_ok; }
    bool hasError()        { return g_mock_update_has_error; }
    const char* errorString() { return g_mock_update_error_str; }
    bool isRunning()       { return true; }
    template<typename T> void printError(T&) {}
};
extern UpdateClass Update;
