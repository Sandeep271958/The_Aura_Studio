#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include "Config.h"
#include "WizController.h"

class WebServerManager {
public:
    WebServerManager(HubState& stateRef, WizController& wizRef);
    void begin();
    void broadcastState();
    void broadcastDiscoveredDevices();
    void cleanupClients();

    // SoftAP Captive Portal & Wi-Fi Provisioning
    void enableCaptivePortal();
    void processDns();
    bool isCaptivePortalActive() const { return captiveActive; }

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    DNSServer dnsServer;
    bool captiveActive = false;
    HubState& state;
    WizController& wiz;

    void setupRoutes();
    void setupWebSocket();
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void handleWsMessage(void *arg, uint8_t *data, size_t len);
    String serializeState();
    String getSetupPortalHtml();
};
