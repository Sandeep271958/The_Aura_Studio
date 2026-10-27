#pragma once

#include "Config.h"
#include "WizController.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class WebServerManager {
public:
  WebServerManager(HubState &stateRef, WizController &wizRef);
  void begin();
  void broadcastState();
  void broadcastDiscoveredDevices();

private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  HubState &state;
  WizController &wiz;

  void setupRoutes();
  void setupWebSocket();
  void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                 AwsEventType type, void *arg, uint8_t *data, size_t len);
  void handleWsMessage(void *arg, uint8_t *data, size_t len);
  String serializeState();
};
