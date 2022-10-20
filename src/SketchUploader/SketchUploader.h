#ifndef SKETCH_UPLOADER_h
#define SKETCH_UPLOADER_h

#include "Arduino.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#include <NTPClient.h>

#include <Update.h>

#include <array>

//#include "SketchUploader_web/login.h"
#include "SketchUploader_web/server.h"

#define USERNAME "dano"
#define PASSWORD "mano"
#define MAX_LOG_MEMORY 15

class SketchUploader {

    public: 
    SketchUploader();

    static AsyncWebServer server;
    static AsyncEventSource events;
    static WiFiUDP ntpUDP;
    static NTPClient timeClient;

    void startServer();

    void log(String str);

    uint8_t logPointer = 0;
    std::array<String, MAX_LOG_MEMORY> lastLogs;
    void retrieveLastLogs();

};

extern SketchUploader SU;

#endif