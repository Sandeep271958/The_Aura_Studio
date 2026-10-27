#include "WebServerManager.h"
#include <LittleFS.h>

WebServerManager::WebServerManager(HubState& stateRef, WizController& wizRef)
    : server(80), ws("/ws"), state(stateRef), wiz(wizRef) {}

void WebServerManager::begin() {
    setupWebSocket();
    setupRoutes();
    server.begin();
    Serial.println("[HTTP] AsyncWebServer & WebSockets running on port 80");
}

void WebServerManager::setupWebSocket() {
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onWsEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);
}

void WebServerManager::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("[WS] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            client->text(serializeState());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("[WS] Client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWsMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebServerManager::handleWsMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        StaticJsonDocument<1024> doc;
        DeserializationError err = deserializeJson(doc, (char*)data);
        if (err) {
            Serial.println("[WS] JSON parse error");
            return;
        }

        String msgType = doc["type"] | "";
        if (msgType == "getState") {
            broadcastState();
        } else if (msgType == "scanWiz") {
            wiz.broadcastDiscovery();
        } else if (msgType == "update") {
            JsonObject payload = doc["payload"];

            if (payload.containsKey("isMasterOn")) {
                state.isMasterOn = payload["isMasterOn"];
            }
            if (payload.containsKey("masterBrightness")) {
                state.masterBrightness = payload["masterBrightness"];
            }
            if (payload.containsKey("contrastRatio")) {
                state.contrastRatio = payload["contrastRatio"];
            }
            if (payload.containsKey("syncCorners")) {
                state.syncCorners = payload["syncCorners"];
            }
            if (payload.containsKey("activePreset")) {
                state.activePreset = payload["activePreset"].as<String>();
            }

            // Desk updates
            if (payload.containsKey("desk")) {
                JsonObject d = payload["desk"];
                if (d.containsKey("power")) state.desk.power = d["power"];
                if (d.containsKey("brightness")) state.desk.brightness = d["brightness"];
                if (d.containsKey("color")) {
                    String hexColor = d["color"].as<String>();
                    if (hexColor.startsWith("#") && hexColor.length() == 7) {
                        long number = strtol(&hexColor[1], NULL, 16);
                        state.desk.r = (number >> 16) & 0xFF;
                        state.desk.g = (number >> 8) & 0xFF;
                        state.desk.b = number & 0xFF;
                    }
                }
                if (d.containsKey("effect")) state.desk.effect = d["effect"].as<String>();
                if (d.containsKey("speed")) state.desk.speed = d["speed"];
                if (d.containsKey("intensity")) state.desk.intensity = d["intensity"];
            }

            // WiZ Left updates
            if (payload.containsKey("wizLeft")) {
                JsonObject wl = payload["wizLeft"];
                if (wl.containsKey("power")) state.wizLeft.power = wl["power"];
                if (wl.containsKey("brightness")) state.wizLeft.brightness = wl["brightness"];
                if (wl.containsKey("mode")) state.wizLeft.mode = wl["mode"].as<String>();
                if (wl.containsKey("kelvin")) state.wizLeft.kelvin = wl["kelvin"];
                if (wl.containsKey("color")) {
                    String hexColor = wl["color"].as<String>();
                    if (hexColor.startsWith("#") && hexColor.length() == 7) {
                        long number = strtol(&hexColor[1], NULL, 16);
                        state.wizLeft.r = (number >> 16) & 0xFF;
                        state.wizLeft.g = (number >> 8) & 0xFF;
                        state.wizLeft.b = number & 0xFF;
                    }
                }
            }

            // WiZ Right updates
            if (payload.containsKey("wizRight")) {
                JsonObject wr = payload["wizRight"];
                if (wr.containsKey("power")) state.wizRight.power = wr["power"];
                if (wr.containsKey("brightness")) state.wizRight.brightness = wr["brightness"];
                if (wr.containsKey("mode")) state.wizRight.mode = wr["mode"].as<String>();
                if (wr.containsKey("kelvin")) state.wizRight.kelvin = wr["kelvin"];
                if (wr.containsKey("color")) {
                    String hexColor = wr["color"].as<String>();
                    if (hexColor.startsWith("#") && hexColor.length() == 7) {
                        long number = strtol(&hexColor[1], NULL, 16);
                        state.wizRight.r = (number >> 16) & 0xFF;
                        state.wizRight.g = (number >> 8) & 0xFF;
                        state.wizRight.b = number & 0xFF;
                    }
                }
            }

            // Sync WiZ bulbs over UDP
            float masterFactor = state.isMasterOn ? (state.masterBrightness / 100.0f) : 0.0f;
            float cornerFactor = 1.0f - ((float)state.contrastRatio / 100.0f);
            if (cornerFactor < 0.0f) cornerFactor = 0.0f;

            uint8_t leftEffectiveDim = constrain((int)(state.wizLeft.brightness * masterFactor * cornerFactor), 10, 100);
            uint8_t rightEffectiveDim = constrain((int)(state.wizRight.brightness * masterFactor * cornerFactor), 10, 100);

            bool leftPower = state.isMasterOn && state.wizLeft.power;
            bool rightPower = state.isMasterOn && state.wizRight.power;

            if (state.wizLeft.mode == "rgb") {
                wiz.sendPilotRGB(state.wizLeftIp, leftPower, state.wizLeft.r, state.wizLeft.g, state.wizLeft.b, leftEffectiveDim);
            } else {
                wiz.sendPilotCCT(state.wizLeftIp, leftPower, state.wizLeft.kelvin, leftEffectiveDim);
            }

            if (state.wizRight.mode == "rgb") {
                wiz.sendPilotRGB(state.wizRightIp, rightPower, state.wizRight.r, state.wizRight.g, state.wizRight.b, rightEffectiveDim);
            } else {
                wiz.sendPilotCCT(state.wizRightIp, rightPower, state.wizRight.kelvin, rightEffectiveDim);
            }

            broadcastState();
        }
    }
}

void WebServerManager::setupRoutes() {
    // Serve static files from LittleFS
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // REST API - Get current state
    server.on("/api/state", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json", serializeState());
    });

    // REST API - Trigger WiZ Scan
    server.on("/api/wiz/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        wiz.broadcastDiscovery();
        request->send(200, "application/json", "{\"status\":\"scanning\"}");
    });

    // 404 Handler with automatic rescue page if LittleFS image has not been uploaded yet
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (!LittleFS.exists("/index.html")) {
            String rescue = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><title>AURA Setup</title><style>body{background:#07090e;color:#f8fafc;font-family:-apple-system,sans-serif;padding:40px;text-align:center}h1{color:#00f2fe;font-size:24px}p{color:#94a3b8;font-size:16px;max-width:500px;margin:12px auto}.box{background:rgba(255,255,255,0.05);border:1px solid rgba(255,255,255,0.1);border-radius:12px;padding:24px;display:inline-block;margin-top:20px}code{background:rgba(0,242,254,0.15);color:#00f2fe;padding:4px 8px;border-radius:6px;font-family:monospace}</style></head><body><h1>AURA // ESP32 Smart Light Hub</h1><div class='box'><p>Firmware is running and online! However, dashboard files are not found on flash memory.</p><p>In PlatformIO, run <code>Upload Filesystem Image</code> to flash the web dashboard to LittleFS.</p></div></body></html>";
            request->send(200, "text/html", rescue);
        } else {
            request->send(404, "text/plain", "AURA Hub: File Not Found");
        }
    });
}

void WebServerManager::cleanupClients() {
    ws.cleanupClients();
}

String WebServerManager::serializeState() {
    StaticJsonDocument<1536> doc;
    doc["type"] = "state";
    JsonObject payload = doc.createNestedObject("payload");

    payload["isMasterOn"] = state.isMasterOn;
    payload["masterBrightness"] = state.masterBrightness;
    payload["contrastRatio"] = state.contrastRatio;
    payload["syncCorners"] = state.syncCorners;
    payload["activePreset"] = state.activePreset;

    JsonObject net = payload.createNestedObject("network");
    net["hubIp"] = WiFi.localIP().toString();
    net["wizLeftIp"] = state.wizLeftIp;
    net["wizRightIp"] = state.wizRightIp;

    JsonObject dev = payload.createNestedObject("devices");

    // Desk
    JsonObject deskObj = dev.createNestedObject("desk");
    deskObj["power"] = state.desk.power;
    deskObj["brightness"] = state.desk.brightness;
    char hexDesk[8];
    snprintf(hexDesk, sizeof(hexDesk), "#%02x%02x%02x", state.desk.r, state.desk.g, state.desk.b);
    deskObj["color"] = hexDesk;
    deskObj["effect"] = state.desk.effect;
    deskObj["speed"] = state.desk.speed;
    deskObj["intensity"] = state.desk.intensity;

    // WiZ Left
    JsonObject wlObj = dev.createNestedObject("wizLeft");
    wlObj["power"] = state.wizLeft.power;
    wlObj["brightness"] = state.wizLeft.brightness;
    wlObj["mode"] = state.wizLeft.mode;
    char hexWl[8];
    snprintf(hexWl, sizeof(hexWl), "#%02x%02x%02x", state.wizLeft.r, state.wizLeft.g, state.wizLeft.b);
    wlObj["color"] = hexWl;
    wlObj["kelvin"] = state.wizLeft.kelvin;

    // WiZ Right
    JsonObject wrObj = dev.createNestedObject("wizRight");
    wrObj["power"] = state.wizRight.power;
    wrObj["brightness"] = state.wizRight.brightness;
    wrObj["mode"] = state.wizRight.mode;
    char hexWr[8];
    snprintf(hexWr, sizeof(hexWr), "#%02x%02x%02x", state.wizRight.r, state.wizRight.g, state.wizRight.b);
    wrObj["color"] = hexWr;
    wrObj["kelvin"] = state.wizRight.kelvin;

    String output;
    serializeJson(doc, output);
    return output;
}

void WebServerManager::broadcastState() {
    ws.textAll(serializeState());
}

void WebServerManager::broadcastDiscoveredDevices() {
    StaticJsonDocument<1024> doc;
    doc["type"] = "wizDiscovery";
    JsonArray devices = doc.createNestedArray("devices");

    for (const auto& b : wiz.getDiscoveredBulbs()) {
        JsonObject obj = devices.createNestedObject();
        obj["ip"] = b.ip;
        obj["mac"] = b.mac;
        obj["model"] = b.model;
        obj["rssi"] = b.rssi;
    }

    String output;
    serializeJson(doc, output);
    ws.textAll(output);
}
