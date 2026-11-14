#include "WizController.h"

WizController::WizController() : udpMutex(NULL) {}

void WizController::begin() {
  if (udpMutex == NULL) {
    udpMutex = xSemaphoreCreateMutex();
  }
  udp.begin(WIZ_UDP_PORT);
  Serial.println("[WiZ] UDP listener initialized on port 38899");
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
  if (udpMutex != NULL && xSemaphoreTake(udpMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char packetBuffer[512];
      int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
      if (len > 0) {
        packetBuffer[len] = '\0';
      }

      Serial.printf("[WiZ UDP RX] <- %s:38899 | %s\n", udp.remoteIP().toString().c_str(), packetBuffer);

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, packetBuffer);
      if (!error && doc["result"].is<JsonObject>()) {
        JsonObject result = doc["result"];
        if (result.containsKey("mac")) {
          String mac = result["mac"] | "unknown";
          String ipStr = udp.remoteIP().toString();

          // Check if already in list
          bool exists = false;
          for (const auto &b : discoveredBulbs) {
            if (b.ip == ipStr) {
              exists = true;
              break;
            }
          }

          if (!exists) {
            DiscoveredBulb bulb;
            bulb.ip = ipStr;
            bulb.mac = mac;
            bulb.model = result["moduleName"] | "Philips WiZ Device";
            bulb.rssi = result["rssi"] | 0;
            discoveredBulbs.push_back(bulb);
            Serial.printf("[WiZ] Discovered bulb: %s (%s) at %s\n",
                          bulb.model.c_str(), mac.c_str(), ipStr.c_str());
          }
        }
      }
    }
    xSemaphoreGive(udpMutex);
  }
}

const std::vector<DiscoveredBulb> &WizController::getDiscoveredBulbs() const {
  return discoveredBulbs;
}

void WizController::clearDiscoveredBulbs() { discoveredBulbs.clear(); }

void WizController::sendUdpPacket(const IPAddress &ip,
                                  const String &jsonPayload) {
  if (udpMutex != NULL) xSemaphoreTake(udpMutex, pdMS_TO_TICKS(100));
  udp.beginPacket(ip, WIZ_UDP_PORT);
  udp.write((const uint8_t *)jsonPayload.c_str(), jsonPayload.length());
  int res = udp.endPacket();
  if (udpMutex != NULL) xSemaphoreGive(udpMutex);
  Serial.printf("[WiZ UDP TX] (status=%d) -> %s:38899 | %s\n", res, ip.toString().c_str(), jsonPayload.c_str());
}

String WizController::sendCommandWithResponse(const String &ipStr, const String &jsonPayload, uint32_t timeoutMs) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) return "{\"error\":\"Invalid IP format\"}";

  if (udpMutex != NULL) xSemaphoreTake(udpMutex, pdMS_TO_TICKS(200));

  // Flush any pending incoming packets
  while (udp.parsePacket() > 0) {
    udp.flush();
  }

  udp.beginPacket(ip, WIZ_UDP_PORT);
  udp.write((const uint8_t *)jsonPayload.c_str(), jsonPayload.length());
  int res = udp.endPacket();
  Serial.printf("[WiZ Test TX] (status=%d) -> %s:38899 | %s\n", res, ipStr.c_str(), jsonPayload.c_str());

  if (res != 1) {
    if (udpMutex != NULL) xSemaphoreGive(udpMutex);
    return "{\"error\":\"UDP endPacket failed (socket send error)\"}";
  }

  // Await response
  uint32_t start = millis();
  String response = "";
  while (millis() - start < timeoutMs) {
    int size = udp.parsePacket();
    if (size > 0) {
      char buf[512];
      int len = udp.read(buf, sizeof(buf) - 1);
      if (len > 0) {
        buf[len] = '\0';
        response = String(buf);
        Serial.printf("[WiZ Test RX] <- %s:38899 | %s\n", udp.remoteIP().toString().c_str(), buf);
        break;
      }
    }
    delay(10);
  }

  if (udpMutex != NULL) xSemaphoreGive(udpMutex);

  if (response.length() == 0) {
    return "{\"error\":\"Timeout - No UDP response from bulb on port 38899. Check if 'Allow local communication' is enabled in WiZ v2 app.\"}";
  }
  return response;
}
