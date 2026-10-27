#include "WizController.h"

WizController::WizController() {}

void WizController::begin() {
  udp.begin(WIZ_UDP_PORT);
  Serial.println("[WiZ] UDP listener initialized on port 38899");
}

void WizController::sendPilotRGB(const String &ipStr, bool state, uint8_t r,
                                 uint8_t g, uint8_t b, uint8_t dimming) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) {
    Serial.printf("[WiZ] Invalid IP format: %s\n", ipStr.c_str());
    return;
  }

  // WiZ expects dimming in range 10-100 (clamp min 10 if on)
  uint8_t wizDim = dimming < 10 ? 10 : (dimming > 100 ? 100 : dimming);

  StaticJsonDocument<256> doc;
  doc["method"] = "setPilot";
  JsonObject params = doc.createNestedObject("params");
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

  // WiZ bulb CCT limits: 2200K - 6500K
  uint16_t clampedK = constrain(kelvin, 2200, 6500);
  uint8_t wizDim = dimming < 10 ? 10 : (dimming > 100 ? 100 : dimming);

  StaticJsonDocument<256> doc;
  doc["method"] = "setPilot";
  JsonObject params = doc.createNestedObject("params");
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

  StaticJsonDocument<128> doc;
  doc["method"] = "setPilot";
  JsonObject params = doc.createNestedObject("params");
  params["state"] = state;

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  sendUdpPacket(ip, jsonPayload);
}

void WizController::broadcastDiscovery() {
  clearDiscoveredBulbs();
  IPAddress broadcastIp(255, 255, 255, 255);
  String query = "{\"method\":\"getSystemConfig\",\"params\":{}}";
  sendUdpPacket(broadcastIp, query);
  Serial.println(
      "[WiZ] Dispatched broadcast discovery query to 255.255.255.255:38899");
}

void WizController::updateDiscovery() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[512];
    int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) {
      packetBuffer[len] = '\0';
    }

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, packetBuffer);
    if (!error && doc.containsKey("result")) {
      JsonObject result = doc["result"];
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

const std::vector<DiscoveredBulb> &WizController::getDiscoveredBulbs() const {
  return discoveredBulbs;
}

void WizController::clearDiscoveredBulbs() { discoveredBulbs.clear(); }

void WizController::sendUdpPacket(const IPAddress &ip,
                                  const String &jsonPayload) {
  udp.beginPacket(ip, WIZ_UDP_PORT);
  udp.write((const uint8_t *)jsonPayload.c_str(), jsonPayload.length());
  udp.endPacket();
}
