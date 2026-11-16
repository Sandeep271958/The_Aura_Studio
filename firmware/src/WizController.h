#pragma once

#include "Config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <vector>

struct DiscoveredBulb {
  String ip;
  String mac;
  String model;
  int rssi;
};

class WizController {
public:
  WizController(HubState* state = nullptr);
  void begin(HubState* state = nullptr);
  void setState(HubState* state);

  // Command dispatchers
  void sendPilotRGB(const String &ipStr, bool state, uint8_t r, uint8_t g,
                    uint8_t b, uint8_t dimming);
  void sendPilotCCT(const String &ipStr, bool state, uint16_t kelvin,
                    uint8_t dimming);
  void sendPower(const String &ipStr, bool state);

  // Diagnostic dispatcher with synchronous response capture
  String sendCommandWithResponse(const String &ipStr, const String &jsonPayload, uint32_t timeoutMs = 800);

  // Auto-discovery & auto-rebind
  void broadcastDiscovery();
  void updateDiscovery();
  void updateHeartbeat();
  std::vector<DiscoveredBulb> getDiscoveredBulbs() const;
  void clearDiscoveredBulbs();

  bool checkAndClearRebound();
  static String normalizeMac(const String &mac);

  // Instant State Synchronization for newly online bulbs
  void syncBulb(int bulbIndex); // 0 = Left, 1 = Right, -1 = Both
  void syncAllBulbs();
  bool isLeftOnline() const { return leftHealth.isOnline; }
  bool isRightOnline() const { return rightHealth.isOnline; }

private:
  WiFiUDP udp;
  mutable SemaphoreHandle_t udpMutex;
  std::vector<DiscoveredBulb> discoveredBulbs;
  HubState* hubState;
  volatile bool reboundOccurred;
  void sendUdpPacket(const IPAddress &ip, const String &jsonPayload);

  struct BulbHealth {
    bool isOnline = false;
    uint32_t lastSeenMs = 0;
    uint32_t pendingResendMs = 0;
    int pendingResendIndex = -1;
  };
  BulbHealth leftHealth;
  BulbHealth rightHealth;
  uint32_t lastHeartbeatPoll = 0;
};
