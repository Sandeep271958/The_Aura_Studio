#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "WizController.h"

class WebServerManager {
public:
    WebServerManager(HubState& stateRef, WizController& wizRef);
    void begin();
    void broadcastState();
    void broadcastDiscoveredDevices();
    void cleanupClients();

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    HubState& state;
    WizController& wiz;

    void setupRoutes();
    void setupWebSocket();
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void handleWsMessage(void *arg, uint8_t *data, size_t len);
    String serializeState();
};
