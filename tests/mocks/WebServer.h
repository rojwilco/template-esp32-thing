#pragma once
#include "Arduino.h"
#include <map>
#include <string>
#include <functional>

// Upload status
enum HTTPUploadStatus { UPLOAD_FILE_START=0, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };

struct HTTPUpload {
    HTTPUploadStatus status = UPLOAD_FILE_END;
    std::string filename;
    uint8_t buf[1436] = {};
    size_t currentSize = 0;
    size_t totalSize   = 0;
};

#define HTTP_GET  0
#define HTTP_POST 1
#define HTTP_ANY  2

// Mock state (defined in mock_support.cpp)
extern std::map<std::string,std::string> g_mock_server_args;
extern int    g_mock_last_send_code;
extern String g_mock_last_send_body;
extern String g_mock_last_redirect;
extern HTTPUpload g_mock_upload;

class WebServer {
    using Handler = std::function<void()>;
    Handler _notFound;
public:
    explicit WebServer(int = 80) {}
    void begin() {}
    void handleClient() {}
    void on(const char*, int, Handler) {}
    void on(const char*, int, Handler, Handler) {}
    void onNotFound(Handler h) { _notFound = h; }

    bool   hasArg(const char* k) { return g_mock_server_args.count(k) > 0; }
    String arg(const char* k) {
        auto it = g_mock_server_args.find(k);
        if(it == g_mock_server_args.end()) return String("");
        return String(it->second.c_str());
    }

    void send(int code, const char* = "", const String& body = String()) {
        g_mock_last_send_code = code;
        g_mock_last_send_body = body;
    }
    void send(int code, const char* ct, const char* body) {
        send(code, ct, String(body));
    }
    void sendHeader(const char* name, const char* value) {
        if(std::string(name) == "Location") g_mock_last_redirect = String(value);
    }

    HTTPUpload& upload() { return g_mock_upload; }
};
