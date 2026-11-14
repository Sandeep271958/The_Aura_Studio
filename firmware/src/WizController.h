#pragma once

#include "Config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

struct DiscoveredBulb {
  String ip;
  String mac;
  String model;
  int rssi;
};

class WizController {
public:
  WizController();
  void begin();

  // Command dispatchers
  void sendPilotRGB(const String &ipStr, bool state, uint8_t r, uint8_t g,
                    uint8_t b, uint8_t dimming);
  void sendPilotCCT(const String &ipStr, bool state, uint16_t kelvin,
                    uint8_t dimming);
  void sendPower(const String &ipStr, bool state);

  // Diagnostic dispatcher with synchronous response capture
  String sendCommandWithResponse(const String &ipStr, const String &jsonPayload, uint32_t timeoutMs = 800);

  // Auto-discovery
  void broadcastDiscovery();
  void updateDiscovery();
  const std::vector<DiscoveredBulb> &getDiscoveredBulbs() const;
  void clearDiscoveredBulbs();

private:
  WiFiUDP udp;
  SemaphoreHandle_t udpMutex;
  std::vector<DiscoveredBulb> discoveredBulbs;
  void sendUdpPacket(const IPAddress &ip, const String &jsonPayload);
};
