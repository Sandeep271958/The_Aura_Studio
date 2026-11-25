#include "WizController.h"
#include <WiFi.h>
#include <Preferences.h>

WizController::WizController(HubState* state) 
  : udpMutex(NULL), hubState(state), reboundOccurred(false) {}

void WizController::begin(HubState* state) {
  if (state != nullptr) {
    hubState = state;
  }
  if (udpMutex == NULL) {
    udpMutex = xSemaphoreCreateMutex();
  }
  udp.begin(WIZ_UDP_PORT);
  Serial.println("[WiZ] UDP listener initialized on port 38899");
}

void WizController::setState(HubState* state) {
  hubState = state;
}

bool WizController::checkAndClearRebound() {
  if (reboundOccurred) {
    reboundOccurred = false;
    return true;
  }
  return false;
}

String WizController::normalizeMac(const String &mac) {
  String clean = "";
  clean.reserve(mac.length());
  for (size_t i = 0; i < mac.length(); i++) {
    char c = mac[i];
    if (c != ':' && c != '-' && c != ' ') {
      clean += (char)tolower((unsigned char)c);
    }
  }
  return clean;
}

void WizController::sendPilotRGB(const String &ipStr, bool state, uint8_t r,
                                 uint8_t g, uint8_t b, uint8_t dimming) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) {
    return;
  }
  if (ipStr == "0.0.0.0" || ipStr.length() < 7) return;

  // WiZ expects dimming in range 10-100 (clamp min 10 if on)
  uint8_t wizDim = dimming < 10 ? 10 : (dimming > 100 ? 100 : dimming);

  JsonDocument doc;
  doc["id"] = 1;
  doc["method"] = "setPilot";
  JsonObject params = doc["params"].to<JsonObject>();
  params["state"] = state;
  if (state) {
    params["r"] = r;
    params["g"] = g;
    params["b"] = b;
    params["dimming"] = wizDim;
  }

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  sendUdpPacket(ip, jsonPayload);
}

void WizController::sendPilotCCT(const String &ipStr, bool state,
                                 uint16_t kelvin, uint8_t dimming) {
  IPAddress ip;
  if (!ip.fromString(ipStr))
    return;
  if (ipStr == "0.0.0.0" || ipStr.length() < 7) return;

  // WiZ bulb CCT limits: 2200K - 6500K
  uint16_t clampedK = constrain(kelvin, 2200, 6500);
  uint8_t wizDim = dimming < 10 ? 10 : (dimming > 100 ? 100 : dimming);

  JsonDocument doc;
  doc["id"] = 1;
  doc["method"] = "setPilot";
  JsonObject params = doc["params"].to<JsonObject>();
  params["state"] = state;
  if (state) {
    params["temp"] = clampedK;
    params["dimming"] = wizDim;
  }

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  sendUdpPacket(ip, jsonPayload);
}

void WizController::sendPower(const String &ipStr, bool state) {
  IPAddress ip;
  if (!ip.fromString(ipStr))
    return;
  if (ipStr == "0.0.0.0" || ipStr.length() < 7) return;

  JsonDocument doc;
  doc["id"] = 1;
  doc["method"] = "setPilot";
  JsonObject params = doc["params"].to<JsonObject>();
  params["state"] = state;

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  sendUdpPacket(ip, jsonPayload);
}

void WizController::broadcastDiscovery() {
  clearDiscoveredBulbs();
  IPAddress broadcastIp(255, 255, 255, 255);
  String query = "{\"id\":1,\"method\":\"getSystemConfig\",\"params\":{}}";
  sendUdpPacket(broadcastIp, query);
  Serial.println(
      "[WiZ] Dispatched broadcast discovery query to 255.255.255.255:38899");
}

void WizController::updateDiscovery() {
  String discoveredMac = "";
  String discoveredIp = "";
  String discoveredModel = "";
  String packetMethod = "";
  int discoveredRssi = 0;
  bool packetValid = false;

  // -------------------------------------------------------------
  // Phase 1: Read UDP packet and maintain discoveredBulbs (udpMutex)
  // -------------------------------------------------------------
  if (udpMutex != NULL && xSemaphoreTake(udpMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
      char packetBuffer[512];
      int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
      if (len > 0) {
        packetBuffer[len] = '\0';

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, packetBuffer);
        if (!error) {
          JsonObject dataObj;
          if (doc["result"].is<JsonObject>()) {
            dataObj = doc["result"].as<JsonObject>();
          } else if (doc["params"].is<JsonObject>()) {
            dataObj = doc["params"].as<JsonObject>();
          }

          packetMethod = doc["method"] | "";

          if (!dataObj.isNull()) {
            discoveredMac = dataObj["mac"] | "";
            discoveredIp = udp.remoteIP().toString();
            discoveredModel = dataObj["moduleName"] | "Philips WiZ Device";
            discoveredRssi = dataObj["rssi"] | 0;
            packetValid = true;

            String cleanDiscoveredMac = normalizeMac(discoveredMac);

            // Update existing or add new bulb in discovered list
            bool exists = false;
            for (auto &b : discoveredBulbs) {
              if ((cleanDiscoveredMac.length() >= 6 && normalizeMac(b.mac) == cleanDiscoveredMac) || b.ip == discoveredIp) {
                exists = true;
                b.ip = discoveredIp;
                if (discoveredMac.length() >= 6) b.mac = discoveredMac;
                b.model = discoveredModel;
                b.rssi = discoveredRssi;
                break;
              }
            }

            if (!exists) {
              if (discoveredBulbs.size() < 32) {
                DiscoveredBulb bulb;
                bulb.ip = discoveredIp;
                bulb.mac = discoveredMac;
                bulb.model = discoveredModel;
                bulb.rssi = discoveredRssi;
                discoveredBulbs.push_back(bulb);
                Serial.printf("[WiZ] Discovered bulb: %s (%s) at %s\n",
                              bulb.model.c_str(), discoveredMac.c_str(), discoveredIp.c_str());
              }
            }
          }
        }
      }
    }
    xSemaphoreGive(udpMutex);
  }

  // -------------------------------------------------------------
  // Phase 2: Check Auto-Rebind under stateMutex (outside udpMutex)
  // -------------------------------------------------------------
  if (packetValid && hubState != nullptr && discoveredMac.length() >= 6) {
    String cleanMac = normalizeMac(discoveredMac);
    bool reboundLeft = false;
    bool reboundRight = false;
    String oldLeftIp = "";
    String oldRightIp = "";

    if (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      // Check Left WiZ Bulb
      if (hubState->wizLeftMac.length() >= 6 && normalizeMac(hubState->wizLeftMac) == cleanMac) {
        if (hubState->wizLeftIp != discoveredIp) {
          oldLeftIp = hubState->wizLeftIp;
          hubState->wizLeftIp = discoveredIp;
          reboundLeft = true;
        }
      }

      // Check Right WiZ Bulb
      if (hubState->wizRightMac.length() >= 6 && normalizeMac(hubState->wizRightMac) == cleanMac) {
        if (hubState->wizRightIp != discoveredIp) {
          oldRightIp = hubState->wizRightIp;
          hubState->wizRightIp = discoveredIp;
          reboundRight = true;
        }
      }

      // If IP changed, persist to NVS Preferences while stateMutex is held
      if (reboundLeft || reboundRight) {
        Preferences prefs;
        if (prefs.begin("aura", false)) {
          if (reboundLeft) {
            prefs.putString("wizLeftIp", discoveredIp);
          }
          if (reboundRight) {
            prefs.putString("wizRightIp", discoveredIp);
          }
          prefs.end();
        }
        reboundOccurred = true;
      }

      xSemaphoreGive(stateMutex);
    }

    if (reboundLeft) {
      Serial.printf("[WiZ Auto-Rebind] Left bulb (%s) IP updated: %s -> %s (Persisted to NVS)\n",
                    discoveredMac.c_str(), oldLeftIp.c_str(), discoveredIp.c_str());
    }
    if (reboundRight) {
      Serial.printf("[WiZ Auto-Rebind] Right bulb (%s) IP updated: %s -> %s (Persisted to NVS)\n",
                    discoveredMac.c_str(), oldRightIp.c_str(), discoveredIp.c_str());
    }
  }

  // -------------------------------------------------------------
  // Phase 3: Immediate Auto-Sync on Bulb Arrival / Cold Boot
  // -------------------------------------------------------------
  if (packetValid && hubState != nullptr) {
    String leftIpCopy, leftMacCopy, rightIpCopy, rightMacCopy;
    bool mutexTaken = (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdTRUE);
    if (mutexTaken || stateMutex == NULL) {
      leftIpCopy = hubState->wizLeftIp;
      leftMacCopy = hubState->wizLeftMac;
      rightIpCopy = hubState->wizRightIp;
      rightMacCopy = hubState->wizRightMac;
      if (mutexTaken) xSemaphoreGive(stateMutex);
    }

    String cleanMac = (discoveredMac.length() >= 6) ? normalizeMac(discoveredMac) : "";
    bool isLeft = (leftIpCopy == discoveredIp) || (cleanMac.length() >= 6 && leftMacCopy.length() >= 6 && normalizeMac(leftMacCopy) == cleanMac);
    bool isRight = (rightIpCopy == discoveredIp) || (cleanMac.length() >= 6 && rightMacCopy.length() >= 6 && normalizeMac(rightMacCopy) == cleanMac);
    bool isFirstBeat = (packetMethod == "firstBeat");

    if (isLeft) {
      bool wasOffline = !leftHealth.isOnline;
      leftHealth.isOnline = true;
      leftHealth.lastSeenMs = millis();
      if (wasOffline || isFirstBeat) {
        Serial.printf("[WiZ Auto-Sync] Left bulb (%s) online (firstBeat=%d, wasOffline=%d) -> Immediately putting bulb into system color!\n",
                      discoveredIp.c_str(), isFirstBeat, wasOffline);
        syncBulb(0);
        leftHealth.pendingResendIndex = 0;
        leftHealth.pendingResendMs = millis() + 300; // Follow-up resend for 100% UDP arrival assurance
      }
    }

    if (isRight) {
      bool wasOffline = !rightHealth.isOnline;
      rightHealth.isOnline = true;
      rightHealth.lastSeenMs = millis();
      if (wasOffline || isFirstBeat) {
        Serial.printf("[WiZ Auto-Sync] Right bulb (%s) online (firstBeat=%d, wasOffline=%d) -> Immediately putting bulb into system color!\n",
                      discoveredIp.c_str(), isFirstBeat, wasOffline);
        syncBulb(1);
        rightHealth.pendingResendIndex = 1;
        rightHealth.pendingResendMs = millis() + 300; // Follow-up resend for 100% UDP arrival assurance
      }
    }
  }
}

void WizController::syncBulb(int bulbIndex) {
  if (hubState == nullptr || WiFi.status() != WL_CONNECTED) return;

  String targetIp = "";
  bool masterOn = true;
  uint8_t masterBri = 80;
  int8_t contrast = 0;
  DeviceWiz dev;

  if (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    masterOn = hubState->isMasterOn;
    masterBri = hubState->masterBrightness;
    contrast = hubState->contrastRatio;
    if (bulbIndex == 0 || bulbIndex == -1) {
      targetIp = hubState->wizLeftIp;
      dev = hubState->wizLeft;
    }
    if (bulbIndex == 1) {
      targetIp = hubState->wizRightIp;
      dev = hubState->wizRight;
    }
    xSemaphoreGive(stateMutex);
  } else {
    return;
  }

  if (targetIp.length() >= 7 && targetIp != "0.0.0.0") {
    float masterFactor = masterOn ? (masterBri / 100.0f) : 0.0f;
    float cornerFactor = 1.0f - ((float)contrast / 100.0f);
    if (cornerFactor < 0.0f) cornerFactor = 0.0f;
    if (cornerFactor > 1.5f) cornerFactor = 1.5f;

    uint8_t effectiveDim = constrain((int)(dev.brightness * masterFactor * cornerFactor), 10, 100);
    bool effectivePower = masterOn && dev.power && (masterBri > 0) && (dev.brightness > 0);

    Serial.printf("[WiZ Sync Dispatch] %s (Bulb %s): Power=%s Mode=%s Dim=%u Color=(%u,%u,%u, %uK)\n",
                  targetIp.c_str(), (bulbIndex == 0 ? "Left" : "Right"), (effectivePower ? "ON" : "OFF"),
                  dev.mode.c_str(), effectiveDim, dev.r, dev.g, dev.b, dev.kelvin);

    if (dev.mode == "rgb") {
      sendPilotRGB(targetIp, effectivePower, dev.r, dev.g, dev.b, effectiveDim);
    } else {
      sendPilotCCT(targetIp, effectivePower, dev.kelvin, effectiveDim);
    }
  }

  if (bulbIndex == -1) {
    syncBulb(1);
  }
}

void WizController::syncAllBulbs() {
  syncBulb(0);
  syncBulb(1);
}

void WizController::updateHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) return;
  uint32_t now = millis();

  // 1. Periodic non-blocking UDP status poll every 2500ms
  if (now - lastHeartbeatPoll >= 2500) {
    lastHeartbeatPoll = now;
    if (hubState != nullptr) {
      if (hubState->wizLeftIp.length() >= 7 && hubState->wizLeftIp != "0.0.0.0") {
        IPAddress ip;
        if (ip.fromString(hubState->wizLeftIp)) {
          sendUdpPacket(ip, "{\"id\":1,\"method\":\"getPilot\",\"params\":{}}");
        }
      }
      if (hubState->wizRightIp.length() >= 7 && hubState->wizRightIp != "0.0.0.0") {
        IPAddress ip;
        if (ip.fromString(hubState->wizRightIp)) {
          sendUdpPacket(ip, "{\"id\":1,\"method\":\"getPilot\",\"params\":{}}");
        }
      }
    }

    // 2. Offline timeout detection (> 6000ms without packet)
    if (leftHealth.isOnline && (now - leftHealth.lastSeenMs > 6000)) {
      leftHealth.isOnline = false;
      Serial.printf("[WiZ Monitor] Left bulb (%s) heartbeat lost -> Marked OFFLINE\n",
                    hubState ? hubState->wizLeftIp.c_str() : "unknown");
    }
    if (rightHealth.isOnline && (now - rightHealth.lastSeenMs > 6000)) {
      rightHealth.isOnline = false;
      Serial.printf("[WiZ Monitor] Right bulb (%s) heartbeat lost -> Marked OFFLINE\n",
                    hubState ? hubState->wizRightIp.c_str() : "unknown");
    }
  }

  // 3. Robust Follow-up Resend (250ms) to ensure UDP arrival while ARP settles
  if (leftHealth.pendingResendIndex >= 0 && now >= leftHealth.pendingResendMs) {
    int idx = leftHealth.pendingResendIndex;
    leftHealth.pendingResendIndex = -1;
    syncBulb(idx);
  }
  if (rightHealth.pendingResendIndex >= 0 && now >= rightHealth.pendingResendMs) {
    int idx = rightHealth.pendingResendIndex;
    rightHealth.pendingResendIndex = -1;
    syncBulb(idx);
  }
}

std::vector<DiscoveredBulb> WizController::getDiscoveredBulbs() const {
  std::vector<DiscoveredBulb> copy;
  if (udpMutex != NULL && xSemaphoreTake(udpMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    copy = discoveredBulbs;
    xSemaphoreGive(udpMutex);
  } else if (udpMutex == NULL) {
    copy = discoveredBulbs;
  }
  return copy;
}

void WizController::clearDiscoveredBulbs() {
  if (udpMutex != NULL && xSemaphoreTake(udpMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    discoveredBulbs.clear();
    xSemaphoreGive(udpMutex);
  } else if (udpMutex == NULL) {
    discoveredBulbs.clear();
  }
}

void WizController::sendUdpPacket(const IPAddress &ip,
                                  const String &jsonPayload) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (udpMutex != NULL && xSemaphoreTake(udpMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    udp.beginPacket(ip, WIZ_UDP_PORT);
    udp.write((const uint8_t *)jsonPayload.c_str(), jsonPayload.length());
    int res = udp.endPacket();
    xSemaphoreGive(udpMutex);
    if (res != 1) {
      Serial.printf("[WiZ UDP TX Error] (status=%d) -> %s:38899\n", res, ip.toString().c_str());
    }
  } else {
    Serial.printf("[WiZ UDP TX] Mutex acquisition failed, dropped packet -> %s:38899\n", ip.toString().c_str());
  }
}

String WizController::sendCommandWithResponse(const String &ipStr, const String &jsonPayload, uint32_t timeoutMs) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) return "{\"error\":\"Invalid IP format\"}";

  bool mutexTaken = false;
  if (udpMutex != NULL) {
    mutexTaken = (xSemaphoreTake(udpMutex, pdMS_TO_TICKS(200)) == pdTRUE);
    if (!mutexTaken) {
      return "{\"error\":\"UDP mutex timeout (busy)\"}";
    }
  } else {
    mutexTaken = true;
  }

  // Flush any pending incoming packets
  while (udp.parsePacket() > 0) {
    char dummy[64];
    while (udp.available() > 0) {
      udp.read(dummy, sizeof(dummy));
    }
  }

  udp.beginPacket(ip, WIZ_UDP_PORT);
  udp.write((const uint8_t *)jsonPayload.c_str(), jsonPayload.length());
  int res = udp.endPacket();
  Serial.printf("[WiZ Test TX] (status=%d) -> %s:38899 | %s\n", res, ipStr.c_str(), jsonPayload.c_str());

  if (res != 1) {
    if (udpMutex != NULL && mutexTaken) xSemaphoreGive(udpMutex);
    return "{\"error\":\"UDP endPacket failed (socket send error)\"}";
  }

  // Await response
  uint32_t start = millis();
  String response = "";
  while (millis() - start < timeoutMs) {
    int size = udp.parsePacket();
    if (size > 0) {
      IPAddress srcIp = udp.remoteIP();
      char buf[512];
      int len = udp.read(buf, sizeof(buf) - 1);
      if (srcIp == ip && len > 0) {
        buf[len] = '\0';
        response = String(buf);
        Serial.printf("[WiZ Test RX] <- %s:38899 | %s\n", srcIp.toString().c_str(), buf);
        break;
      }
    }
    delay(10);
  }

  if (udpMutex != NULL && mutexTaken) xSemaphoreGive(udpMutex);

  if (response.length() == 0) {
    return "{\"error\":\"Timeout - No UDP response from bulb on port 38899. Check if 'Allow local communication' is enabled in WiZ v2 app.\"}";
  }
  return response;
}
