#include "WebServerManager.h"
#include <LittleFS.h>
#include <Preferences.h>
#include <Update.h>

static Preferences prefs;

WebServerManager::WebServerManager(HubState& stateRef, WizController& wizRef)
    : server(80), ws("/ws"), state(stateRef), wiz(wizRef) {}

void WebServerManager::loadPersistentState() {
    prefs.begin("aura", false);
    state.wizLeftIp = prefs.getString("wizLeftIp", DEFAULT_WIZ_LEFT_IP);
    state.wizLeftMac = prefs.getString("wizLeftMac", "");
    state.wizRightIp = prefs.getString("wizRightIp", DEFAULT_WIZ_RIGHT_IP);
    state.wizRightMac = prefs.getString("wizRightMac", "");
    state.useGammaCorrection = prefs.getBool("useGamma", true);
    state.onOffAnimation = prefs.getString("onOffAnim", "gaussian-ripple");

    // Core Hub State
    state.isMasterOn = prefs.getBool("masterOn", true);
    state.masterBrightness = prefs.getUChar("masterBri", 80);
    state.contrastRatio = prefs.getChar("contrast", 0);
    state.syncCorners = prefs.getBool("syncCorners", true);
    state.activePreset = prefs.getString("preset", "cyberpunk");

    // Desk WS2812B Hardware Strip State
    state.desk.power = prefs.getBool("deskPower", true);
    state.desk.brightness = prefs.getUChar("deskBri", 85);
    state.desk.r = prefs.getUChar("deskR", 236);
    state.desk.g = prefs.getUChar("deskG", 72);
    state.desk.b = prefs.getUChar("deskB", 153);
    state.desk.effect = prefs.getString("deskEffect", "cyberpulse");
    state.desk.speed = prefs.getUChar("deskSpeed", 60);
    state.desk.intensity = prefs.getUChar("deskIntens", 75);
    state.desk.hasGradient = prefs.getBool("deskHasGrad", false);

    String stopsStr = prefs.getString("deskStops", "");
    if (stopsStr.length() >= 27) {
        int sIdx = 0;
        int start = 0;
        while (sIdx < 4 && start < (int)stopsStr.length()) {
            int comma = stopsStr.indexOf(',', start);
            String hex = (comma == -1) ? stopsStr.substring(start) : stopsStr.substring(start, comma);
            hex.trim();
            if (hex.startsWith("#") && hex.length() == 7) {
                long val = strtol(&hex[1], NULL, 16);
                state.desk.stops[sIdx] = CRGB((val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
                sIdx++;
            }
            if (comma == -1) break;
            start = comma + 1;
        }
        if (sIdx == 4) state.desk.hasGradient = true;
    }

    // WiZ Left Bulb State
    state.wizLeft.power = prefs.getBool("wlPower", true);
    state.wizLeft.brightness = prefs.getUChar("wlBri", 75);
    state.wizLeft.mode = prefs.getString("wlMode", "rgb");
    state.wizLeft.r = prefs.getUChar("wlR", 30);
    state.wizLeft.g = prefs.getUChar("wlG", 64);
    state.wizLeft.b = prefs.getUChar("wlB", 175);
    state.wizLeft.kelvin = prefs.getUShort("wlK", 2700);

    // WiZ Right Bulb State
    state.wizRight.power = prefs.getBool("wrPower", true);
    state.wizRight.brightness = prefs.getUChar("wrBri", 75);
    state.wizRight.mode = prefs.getString("wrMode", "rgb");
    state.wizRight.r = prefs.getUChar("wrR", 30);
    state.wizRight.g = prefs.getUChar("wrG", 64);
    state.wizRight.b = prefs.getUChar("wrB", 175);
    state.wizRight.kelvin = prefs.getUShort("wrK", 2700);

    Serial.println("[NVS] ===================================================");
    Serial.printf("[NVS] Restored Power State: Master=%s | Desk=%s\n",
                  state.isMasterOn ? "ON" : "OFF", state.desk.power ? "ON" : "OFF");
    Serial.printf("[NVS] Restored Desk Strip: Mode=%s | Dim=%u | Color=#%02x%02x%02x\n",
                  state.desk.effect.c_str(), state.desk.brightness, state.desk.r, state.desk.g, state.desk.b);
    Serial.printf("[NVS] Restored WiZ Left: IP=%s MAC=%s | Power=%s Dim=%u\n",
                  state.wizLeftIp.c_str(), state.wizLeftMac.c_str(), state.wizLeft.power ? "ON" : "OFF", state.wizLeft.brightness);
    Serial.printf("[NVS] Restored WiZ Right: IP=%s MAC=%s | Power=%s Dim=%u\n",
                  state.wizRightIp.c_str(), state.wizRightMac.c_str(), state.wizRight.power ? "ON" : "OFF", state.wizRight.brightness);
    Serial.printf("[NVS] Restored Gamma Correction: %s | On/Off Anim: %s\n",
                  state.useGammaCorrection ? "ON" : "OFF", state.onOffAnimation.c_str());
    Serial.println("[NVS] ===================================================");
}

void WebServerManager::commitStateToNvs() {
    HubState localState;
    if (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        localState = state;
        xSemaphoreGive(stateMutex);
    } else {
        localState = state;
    }

    prefs.putBool("masterOn", localState.isMasterOn);
    prefs.putUChar("masterBri", localState.masterBrightness);
    prefs.putChar("contrast", localState.contrastRatio);
    prefs.putBool("syncCorners", localState.syncCorners);
    prefs.putString("preset", localState.activePreset);
    prefs.putBool("useGamma", localState.useGammaCorrection);
    prefs.putString("onOffAnim", localState.onOffAnimation);

    // WiZ Network Config
    prefs.putString("wizLeftIp", localState.wizLeftIp);
    prefs.putString("wizLeftMac", localState.wizLeftMac);
    prefs.putString("wizRightIp", localState.wizRightIp);
    prefs.putString("wizRightMac", localState.wizRightMac);

    // Desk WS2812B Hardware Strip
    prefs.putBool("deskPower", localState.desk.power);
    prefs.putUChar("deskBri", localState.desk.brightness);
    prefs.putUChar("deskR", localState.desk.r);
    prefs.putUChar("deskG", localState.desk.g);
    prefs.putUChar("deskB", localState.desk.b);
    prefs.putString("deskEffect", localState.desk.effect);
    prefs.putUChar("deskSpeed", localState.desk.speed);
    prefs.putUChar("deskIntens", localState.desk.intensity);
    prefs.putBool("deskHasGrad", localState.desk.hasGradient);

    if (localState.desk.hasGradient) {
        char sBuf[36];
        snprintf(sBuf, sizeof(sBuf), "#%02x%02x%02x,#%02x%02x%02x,#%02x%02x%02x,#%02x%02x%02x",
                 localState.desk.stops[0].r, localState.desk.stops[0].g, localState.desk.stops[0].b,
                 localState.desk.stops[1].r, localState.desk.stops[1].g, localState.desk.stops[1].b,
                 localState.desk.stops[2].r, localState.desk.stops[2].g, localState.desk.stops[2].b,
                 localState.desk.stops[3].r, localState.desk.stops[3].g, localState.desk.stops[3].b);
        prefs.putString("deskStops", sBuf);
    }

    // Left WiZ Bulb
    prefs.putBool("wlPower", localState.wizLeft.power);
    prefs.putUChar("wlBri", localState.wizLeft.brightness);
    prefs.putString("wlMode", localState.wizLeft.mode);
    prefs.putUChar("wlR", localState.wizLeft.r);
    prefs.putUChar("wlG", localState.wizLeft.g);
    prefs.putUChar("wlB", localState.wizLeft.b);
    prefs.putUShort("wlK", localState.wizLeft.kelvin);

    // Right WiZ Bulb
    prefs.putBool("wrPower", localState.wizRight.power);
    prefs.putUChar("wrBri", localState.wizRight.brightness);
    prefs.putString("wrMode", localState.wizRight.mode);
    prefs.putUChar("wrR", localState.wizRight.r);
    prefs.putUChar("wrG", localState.wizRight.g);
    prefs.putUChar("wrB", localState.wizRight.b);
    prefs.putUShort("wrK", localState.wizRight.kelvin);

    Serial.println("[NVS] Full system state persisted to flash memory.");
}

void WebServerManager::savePersistentState(bool immediate) {
    if (immediate) {
        commitStateToNvs();
        nvsDirty = false;
    } else {
        nvsDirty = true;
        lastNvsDirtyTime = millis();
    }
}

void WebServerManager::updatePersistence() {
    if (nvsDirty && (millis() - lastNvsDirtyTime >= 1200)) {
        commitStateToNvs();
        nvsDirty = false;
    }
}

void WebServerManager::begin() {
    loadPersistentState();
    setupWebSocket();
    setupRoutes();
    server.begin();
    Serial.println("[HTTP] AsyncWebServer & WebSockets running on port 80");
}

void WebServerManager::enableCaptivePortal() {
    captiveActive = true;
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    Serial.println("[DNS] Captive Portal DNS responder active on port 53 -> 192.168.4.1");
}

void WebServerManager::processDns() {
    if (captiveActive) {
        dnsServer.processNextRequest();
    }
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
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, (const char*)data, len);
        if (err) {
            Serial.printf("[WS] JSON parse error: %s\n", err.c_str());
            return;
        }

        String msgType = doc["type"] | "";
        if (msgType == "getState") {
            broadcastState();
        } else if (msgType == "scanWiz") {
            wiz.broadcastDiscovery();
        } else if (msgType == "update") {
            JsonObject payload = doc["payload"];
            if (payload.containsKey("state")) {
                payload = payload["state"];
            }

            bool mutexTaken = false;
            if (stateMutex != NULL) {
                mutexTaken = (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE);
            } else {
                mutexTaken = true;
            }

            if (!mutexTaken) {
                Serial.println("[WS] Warning: stateMutex acquisition timed out (100ms), skipping unsynchronized update");
                return;
            }

            if (payload.containsKey("network")) {
                JsonObject net = payload["network"];
                if (net.containsKey("wizLeftIp")) {
                    state.wizLeftIp = net["wizLeftIp"].as<String>();
                    prefs.putString("wizLeftIp", state.wizLeftIp);
                }
                if (net.containsKey("wizLeftMac")) {
                    state.wizLeftMac = net["wizLeftMac"].as<String>();
                    prefs.putString("wizLeftMac", state.wizLeftMac);
                }
                if (net.containsKey("wizRightIp")) {
                    state.wizRightIp = net["wizRightIp"].as<String>();
                    prefs.putString("wizRightIp", state.wizRightIp);
                }
                if (net.containsKey("wizRightMac")) {
                    state.wizRightMac = net["wizRightMac"].as<String>();
                    prefs.putString("wizRightMac", state.wizRightMac);
                }
                Serial.printf("[NVS] Persisted WiZ -> Left: %s (%s) | Right: %s (%s)\n",
                              state.wizLeftIp.c_str(), state.wizLeftMac.c_str(),
                              state.wizRightIp.c_str(), state.wizRightMac.c_str());
            }

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
                state.desk.hasGradient = false;
            }
            if (payload.containsKey("useGammaCorrection")) {
                state.useGammaCorrection = payload["useGammaCorrection"].as<bool>();
                prefs.putBool("useGamma", state.useGammaCorrection);
                Serial.printf("[NVS] Persisted Gamma Correction: %s\n", state.useGammaCorrection ? "ON" : "OFF");
            }
            if (payload.containsKey("onOffAnimation")) {
                state.onOffAnimation = payload["onOffAnimation"].as<String>();
                prefs.putString("onOffAnim", state.onOffAnimation);
                Serial.printf("[NVS] Persisted On/Off Animation: %s\n", state.onOffAnimation.c_str());
            }

            // Desk updates (support flat or nested devices.desk)
            JsonObject d;
            if (payload.containsKey("desk")) {
                d = payload["desk"];
            } else if (payload.containsKey("devices") && payload["devices"].containsKey("desk")) {
                d = payload["devices"]["desk"];
            }

            if (!d.isNull()) {
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
                if (d.containsKey("effect")) {
                    state.desk.effect = d["effect"].as<String>();
                    if (state.desk.effect == "solid" || state.desk.effect == "cyberpulse" ||
                        state.desk.effect == "sunset-wave" || state.desk.effect == "aurora" ||
                        state.desk.effect == "fire" || state.desk.effect == "test-cycle") {
                        state.desk.hasGradient = false;
                    }
                }
                if (d.containsKey("speed")) state.desk.speed = d["speed"];
                if (d.containsKey("intensity")) state.desk.intensity = d["intensity"];
                if (d.containsKey("turbulence")) state.desk.intensity = d["turbulence"];

                if (d.containsKey("gradientStops") && d["gradientStops"].is<JsonArray>()) {
                    JsonArray stopsArr = d["gradientStops"].as<JsonArray>();
                    int stopIdx = 0;
                    for (JsonVariant stopVal : stopsArr) {
                        if (stopIdx >= 4) break;
                        String hexStop = stopVal.as<String>();
                        if (hexStop.startsWith("#") && hexStop.length() == 7) {
                            long number = strtol(&hexStop[1], NULL, 16);
                            uint8_t r = (number >> 16) & 0xFF;
                            uint8_t g = (number >> 8) & 0xFF;
                            uint8_t b = number & 0xFF;
                            state.desk.stops[stopIdx] = CRGB(r, g, b);
                            stopIdx++;
                        }
                    }
                    if (stopIdx == 4) {
                        state.desk.hasGradient = true;
                    }
                }
            }

            // WiZ Left updates (support flat or nested devices.wizLeft)
            JsonObject wl;
            if (payload.containsKey("wizLeft")) {
                wl = payload["wizLeft"];
            } else if (payload.containsKey("devices") && payload["devices"].containsKey("wizLeft")) {
                wl = payload["devices"]["wizLeft"];
            }

            if (!wl.isNull()) {
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

            // WiZ Right updates (support flat or nested devices.wizRight)
            JsonObject wr;
            if (payload.containsKey("wizRight")) {
                wr = payload["wizRight"];
            } else if (payload.containsKey("devices") && payload["devices"].containsKey("wizRight")) {
                wr = payload["devices"]["wizRight"];
            }

            if (!wr.isNull()) {
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

            // Capture local copy for UDP dispatch while mutex is held
            String leftIp = state.wizLeftIp;
            String rightIp = state.wizRightIp;
            bool leftModeRgb = (state.wizLeft.mode == "rgb");
            bool rightModeRgb = (state.wizRight.mode == "rgb");
            uint8_t leftR = state.wizLeft.r, leftG = state.wizLeft.g, leftB = state.wizLeft.b;
            uint8_t rightR = state.wizRight.r, rightG = state.wizRight.g, rightB = state.wizRight.b;
            uint16_t leftK = state.wizLeft.kelvin, rightK = state.wizRight.kelvin;

            float masterFactor = state.isMasterOn ? (state.masterBrightness / 100.0f) : 0.0f;
            float cornerFactor = 1.0f - ((float)state.contrastRatio / 100.0f);
            if (cornerFactor < 0.0f) cornerFactor = 0.0f;
            if (cornerFactor > 1.5f) cornerFactor = 1.5f;

            uint8_t leftEffectiveDim = constrain((int)(state.wizLeft.brightness * masterFactor * cornerFactor), 10, 100);
            uint8_t rightEffectiveDim = constrain((int)(state.wizRight.brightness * masterFactor * cornerFactor), 10, 100);

            bool leftPower = state.isMasterOn && state.wizLeft.power && (state.masterBrightness > 0) && (state.wizLeft.brightness > 0);
            bool rightPower = state.isMasterOn && state.wizRight.power && (state.masterBrightness > 0) && (state.wizRight.brightness > 0);

            bool isDiscreteSetting = payload.containsKey("isMasterOn") ||
                                     payload.containsKey("activePreset") ||
                                     payload.containsKey("network") ||
                                     payload.containsKey("onOffAnimation") ||
                                     payload.containsKey("useGammaCorrection") ||
                                     payload.containsKey("contrastRatio") ||
                                     payload.containsKey("syncCorners") ||
                                     payload.containsKey("masterBrightness");

            if (stateMutex != NULL && mutexTaken) xSemaphoreGive(stateMutex);

            // Sync WiZ bulbs over UDP outside mutex with pacing guard (avoids Wi-Fi congestion & buffer overflow)
            static uint32_t lastLeftWizTx = 0;
            static uint32_t lastRightWizTx = 0;
            static bool lastLeftPower = false;
            static bool lastRightPower = false;
            static uint8_t lastLeftDim = 0;
            static uint8_t lastRightDim = 0;
            static uint8_t lastLeftR = 0, lastLeftG = 0, lastLeftB = 0;
            static uint8_t lastRightR = 0, lastRightG = 0, lastRightB = 0;
            static uint16_t lastLeftK = 0;
            static uint16_t lastRightK = 0;

            uint32_t nowMs = millis();
            bool leftStateChanged = (leftPower != lastLeftPower) ||
                                    (leftModeRgb && (leftR != lastLeftR || leftG != lastLeftG || leftB != lastLeftB)) ||
                                    (!leftModeRgb && leftK != lastLeftK) ||
                                    (abs((int)leftEffectiveDim - (int)lastLeftDim) >= 2);

            if (leftPower != lastLeftPower || (leftStateChanged && (nowMs - lastLeftWizTx >= 80)) || (nowMs - lastLeftWizTx > 4000)) {
                lastLeftWizTx = nowMs;
                lastLeftPower = leftPower;
                lastLeftDim = leftEffectiveDim;
                lastLeftR = leftR; lastLeftG = leftG; lastLeftB = leftB;
                lastLeftK = leftK;
                if (leftModeRgb) {
                    wiz.sendPilotRGB(leftIp, leftPower, leftR, leftG, leftB, leftEffectiveDim);
                } else {
                    wiz.sendPilotCCT(leftIp, leftPower, leftK, leftEffectiveDim);
                }
            }

            bool rightStateChanged = (rightPower != lastRightPower) ||
                                     (rightModeRgb && (rightR != lastRightR || rightG != lastRightG || rightB != lastRightB)) ||
                                     (!rightModeRgb && rightK != lastRightK) ||
                                     (abs((int)rightEffectiveDim - (int)lastRightDim) >= 2);

            if (rightPower != lastRightPower || (rightStateChanged && (nowMs - lastRightWizTx >= 80)) || (nowMs - lastRightWizTx > 4000)) {
                lastRightWizTx = nowMs;
                lastRightPower = rightPower;
                lastRightDim = rightEffectiveDim;
                lastRightR = rightR; lastRightG = rightG; lastRightB = rightB;
                lastRightK = rightK;
                if (rightModeRgb) {
                    wiz.sendPilotRGB(rightIp, rightPower, rightR, rightG, rightB, rightEffectiveDim);
                } else {
                    wiz.sendPilotCCT(rightIp, rightPower, rightK, rightEffectiveDim);
                }
            }

            // Broadcast state only on discrete setting changes or at a throttled 2s heartbeat
            // (Prevents flooding AsyncWebSocket during high-frequency streaming like SoundSync / Ambilight)
            static uint32_t lastBroadcastTime = 0;
            if (isDiscreteSetting || (nowMs - lastBroadcastTime >= 2000)) {
                lastBroadcastTime = nowMs;
                broadcastState();
            }

            // Full State Persistence across Power Cycles (Immediate on power toggle, debounced on sliders)
            bool isImmediateChange = payload.containsKey("isMasterOn") ||
                                     payload.containsKey("activePreset") ||
                                     (payload.containsKey("desk") && payload["desk"].containsKey("power")) ||
                                     (payload.containsKey("devices") && payload["devices"].containsKey("desk") && payload["devices"]["desk"].containsKey("power")) ||
                                     (payload.containsKey("wizLeft") && payload["wizLeft"].containsKey("power")) ||
                                     (payload.containsKey("devices") && payload["devices"].containsKey("wizLeft") && payload["devices"]["wizLeft"].containsKey("power")) ||
                                     (payload.containsKey("wizRight") && payload["wizRight"].containsKey("power")) ||
                                     (payload.containsKey("devices") && payload["devices"].containsKey("wizRight") && payload["devices"]["wizRight"].containsKey("power"));

            savePersistentState(isImmediateChange);
        }
    }
}

void WebServerManager::setupRoutes() {
    // Serve static files from LittleFS with Cache-Control to prevent stale client caching
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html").setCacheControl("no-cache, no-store, must-revalidate");

    // REST API - Get current state
    server.on("/api/state", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json", serializeState());
    });

    // REST API - Trigger WiZ Scan
    server.on("/api/wiz/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        wiz.broadcastDiscovery();
        request->send(200, "application/json", "{\"status\":\"scanning\"}");
    });

    // REST API - Run instant LED Hardware Test Pattern
    server.on("/api/led/test", HTTP_GET, [this](AsyncWebServerRequest *request) {
        bool taken = false;
        if (stateMutex != NULL) {
            taken = (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE);
        } else {
            taken = true;
        }

        if (taken) {
            state.isMasterOn = true;
            state.masterBrightness = 100;
            state.desk.power = true;
            state.desk.brightness = 100;
            state.desk.inspectIndex = -1;
            state.desk.effect = "test-cycle";
            if (stateMutex != NULL) xSemaphoreGive(stateMutex);
            broadcastState();
            request->send(200, "application/json", "{\"status\":\"active\",\"message\":\"Hardware test pattern activated (Red-Green-Blue-White-Chase)\"}");
        } else {
            request->send(503, "application/json", "{\"status\":\"error\",\"message\":\"stateMutex timeout\"}");
        }
    });

    // REST API - Inspect single LED from index 0 to 59 (or -1 to reset to normal)
    server.on("/api/led/inspect", HTTP_GET, [this](AsyncWebServerRequest *request) {
        int idx = request->hasParam("index") ? request->getParam("index")->value().toInt() : -1;
        uint8_t r = request->hasParam("r") ? request->getParam("r")->value().toInt() : 255;
        uint8_t g = request->hasParam("g") ? request->getParam("g")->value().toInt() : 255;
        uint8_t b = request->hasParam("b") ? request->getParam("b")->value().toInt() : 255;

        bool taken = false;
        if (stateMutex != NULL) {
            taken = (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE);
        } else {
            taken = true;
        }

        if (taken) {
            state.isMasterOn = true;
            state.desk.power = true;
            state.desk.inspectIndex = idx;
            if (idx >= 0) {
                state.desk.r = r;
                state.desk.g = g;
                state.desk.b = b;
            }
            if (stateMutex != NULL) xSemaphoreGive(stateMutex);

            broadcastState();
            char msg[128];
            snprintf(msg, sizeof(msg), "{\"status\":\"ok\",\"inspectIndex\":%d,\"r\":%d,\"g\":%d,\"b\":%d}", idx, r, g, b);
            request->send(200, "application/json", msg);
        } else {
            request->send(503, "application/json", "{\"status\":\"error\",\"message\":\"stateMutex timeout\"}");
        }
    });

    // Web OTA Firmware Update Interface (Enables 100% wireless flashing from any browser!)
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>The Aura Studio // Wireless Dual-Target OTA</title><style>*{box-sizing:border-box;margin:0;padding:0}body{background:#07090e;color:#f8fafc;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px}.card{background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.08);backdrop-filter:blur(20px);border-radius:20px;padding:36px 32px;width:100%;max-width:460px;text-align:center;box-shadow:0 25px 50px -12px rgba(0,0,0,0.5)}.badge{display:inline-block;padding:4px 12px;background:rgba(0,242,254,0.1);border:1px solid rgba(0,242,254,0.25);border-radius:9999px;font-size:11px;font-weight:600;letter-spacing:1.5px;color:#00f2fe;text-transform:uppercase;margin-bottom:16px}h1{font-size:22px;font-weight:700;color:#ffffff;margin-bottom:8px;letter-spacing:-0.5px}p{font-size:13px;color:#94a3b8;line-height:1.5;margin-bottom:24px}.drop-zone{border:2px dashed rgba(255,255,255,0.15);border-radius:14px;padding:24px 16px;cursor:pointer;transition:all 0.2s;margin-bottom:20px;background:rgba(255,255,255,0.01)}.drop-zone:hover{border-color:#00f2fe;background:rgba(0,242,254,0.02)}.file-name{font-size:13px;color:#00f2fe;font-family:monospace;margin-top:8px;word-break:break-all}input[type=file]{display:none}.btn{width:100%;padding:14px;background:linear-gradient(135deg,#00f2fe,#a855f7);color:#07090e;border:none;border-radius:12px;font-size:14px;font-weight:700;letter-spacing:0.5px;cursor:pointer;transition:opacity 0.2s}.btn:hover{opacity:0.9}.btn:disabled{opacity:0.4;cursor:not-allowed}.progress-wrap{display:none;margin-top:20px}.progress-bar-bg{width:100%;height:8px;background:rgba(255,255,255,0.08);border-radius:999px;overflow:hidden}.progress-bar{height:100%;width:0%;background:linear-gradient(90deg,#00f2fe,#a855f7);border-radius:999px;transition:width 0.15s}.status-text{font-size:12px;color:#94a3b8;margin-top:10px;font-weight:500}.back-link{display:inline-block;margin-top:20px;font-size:12px;color:#64748b;text-decoration:none;transition:color 0.2s}.back-link:hover{color:#00f2fe}</style></head><body><div class='card'><div class='badge'>Wireless Dual-Target OTA</div><h1>Flash Hub Update</h1><p>Upload <code>firmware_*.bin</code> for C++ logic or <code>littlefs_*.bin</code> for Web Dashboard UI.</p><div class='drop-zone' onclick=\"document.getElementById('fileInput').click()\"><div style='font-size:24px;margin-bottom:6px'>⚡</div><div style='font-size:13px;font-weight:600'>Click to choose .bin file</div><div id='fileName' class='file-name'>No file selected</div></div><input type='file' id='fileInput' accept='.bin' onchange='onFileChosen(this)'><button id='uploadBtn' class='btn' disabled onclick='startUpload()'>Upload & Flash Over-The-Air</button><div id='progressWrap' class='progress-wrap'><div class='progress-bar-bg'><div id='progressBar' class='progress-bar'></div></div><div id='statusText' class='status-text'>Uploading... 0%</div></div><a href='/' class='back-link'>← Return to Dashboard</a></div><script>let chosenFile=null;function onFileChosen(input){if(input.files&&input.files[0]){chosenFile=input.files[0];let target=chosenFile.name.toLowerCase().includes('littlefs')?'Web Dashboard (LittleFS)':'C++ Firmware (Dual OTA)';document.getElementById('fileName').innerText=chosenFile.name+' ['+target+']';document.getElementById('uploadBtn').disabled=false}}function startUpload(){if(!chosenFile)return;const btn=document.getElementById('uploadBtn');const wrap=document.getElementById('progressWrap');const bar=document.getElementById('progressBar');const txt=document.getElementById('statusText');btn.disabled=true;wrap.style.display='block';const formData=new FormData();formData.append('update',chosenFile);const xhr=new XMLHttpRequest();xhr.open('POST','/update',true);xhr.upload.onprogress=function(e){if(e.lengthComputable){const pct=Math.round((e.loaded/e.total)*100);bar.style.width=pct+'%';txt.innerText='Uploading... '+pct+'%'}};xhr.onload=function(){if(xhr.status===200&&xhr.responseText.includes('OK')){bar.style.width='100%';txt.style.color='#00f2fe';let sec=6;txt.innerText='Flash Complete! Rebooting in '+sec+'s...';const timer=setInterval(()=>{sec--;if(sec>0){txt.innerText='Flash Complete! Rebooting in '+sec+'s...'}else{clearInterval(timer);txt.innerText='Reconnecting to Dashboard...';window.location.href='/'}},1000)}else{txt.style.color='#ef4444';txt.innerText='Flash Failed: '+(xhr.responseText||'Server error');btn.disabled=false}};xhr.onerror=function(){txt.style.color='#00f2fe';txt.innerText='Upload complete. Reconnecting in 5s...';setTimeout(()=>{window.location.href='/'},5000)};xhr.send(formData)}</script></body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool shouldReboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
        if (shouldReboot) {
            // Give TCP stack 1500ms to flush 200 OK cleanly before restarting
            xTaskCreate([](void*) {
                vTaskDelay(pdMS_TO_TICKS(1500));
                ESP.restart();
            }, "ota_reboot", 2048, NULL, 1, NULL);
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.printf("[OTA] Update Start: %s\n", filename.c_str());
            int cmd = U_FLASH;
            String lower = filename;
            lower.toLowerCase();
            if (lower.indexOf("littlefs") >= 0 || lower.indexOf("spiffs") >= 0 || lower.indexOf("filesystem") >= 0 || lower.indexOf("data") >= 0) {
                cmd = U_SPIFFS;
                Serial.println("[OTA] Target Partition: LittleFS Filesystem (U_SPIFFS)");
            } else {
                Serial.println("[OTA] Target Partition: Firmware Flash (U_FLASH)");
            }
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }
        if (final) {
            if (Update.end(true)) {
                Serial.printf("[OTA] Update Success: %u bytes\n", index + len);
            } else {
                Update.printError(Serial);
            }
        }
    });

    // REST API - Direct WiZ Diagnostic Test with immediate response
    server.on("/api/wiz/test", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String ip = request->hasParam("ip") ? request->getParam("ip")->value() : state.wizRightIp;
        String action = request->hasParam("action") ? request->getParam("action")->value() : "toggle";

        String payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"dimming\":100}}";
        if (action == "off") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":false}}";
        } else if (action == "on") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"dimming\":100}}";
        } else if (action == "warm") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"temp\":2700,\"dimming\":100}}";
        } else if (action == "cool") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"temp\":6000,\"dimming\":100}}";
        } else if (action == "red") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"r\":255,\"g\":0,\"b\":0,\"dimming\":100}}";
        } else if (action == "green") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"r\":0,\"g\":255,\"b\":0,\"dimming\":100}}";
        } else if (action == "blue") {
            payload = "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"r\":0,\"g\":0,\"b\":255,\"dimming\":100}}";
        }

        String bulbResp = wiz.sendCommandWithResponse(ip, payload);

        JsonDocument resDoc;
        resDoc["targetIp"] = ip;
        resDoc["sentCommand"] = payload;
        resDoc["rawBulbResponse"] = bulbResp;

        String out;
        serializeJson(resDoc, out);
        request->send(200, "application/json", out);
    });

    // Autonomous Captive Portal & Network Provisioning Routes
    server.on("/setup", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", getSetupPortalHtml());
    });

    server.on("/hotspot-detect.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (captiveActive) request->redirect("http://192.168.4.1/setup");
        else request->redirect("/");
    });

    server.on("/canonical.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (captiveActive) request->redirect("http://192.168.4.1/setup");
        else request->redirect("/");
    });

    server.on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (captiveActive) request->redirect("http://192.168.4.1/setup");
        else request->redirect("/");
    });

    server.on("/gen_204", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (captiveActive) request->redirect("http://192.168.4.1/setup");
        else request->redirect("/");
    });

    server.on("/ncsi.txt", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    server.on("/connecttest.txt", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (captiveActive) request->redirect("http://192.168.4.1/setup");
        else request->send(200, "text/plain", "Microsoft Connect Test");
    });

    // REST API - Scan 2.4GHz Wi-Fi networks
    server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        int n = WiFi.scanNetworks(false, true);
        JsonDocument doc;
        JsonArray arr = doc["networks"].to<JsonArray>();
        for (int i = 0; i < n; i++) {
            String s = WiFi.SSID(i);
            if (s.length() == 0) continue;
            JsonObject obj = arr.add<JsonObject>();
            obj["ssid"] = s;
            obj["rssi"] = WiFi.RSSI(i);
            obj["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        }
        WiFi.scanDelete();
        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    // REST API - Current Wi-Fi Status
    server.on("/api/wifi/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        bool isConn = (WiFi.status() == WL_CONNECTED);
        doc["status"] = isConn ? "connected" : (captiveActive ? "setup_mode" : "disconnected");
        doc["ssid"] = isConn ? WiFi.SSID() : state.wifiSsid;
        doc["rssi"] = isConn ? WiFi.RSSI() : 0;
        doc["ip"] = isConn ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        doc["isSetupMode"] = captiveActive || state.isSetupMode;
        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    // REST API - Save Wi-Fi Credentials & Connect
    server.on("/api/wifi/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, (const char*)data, len);
        if (err) {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON payload\"}");
            return;
        }

        String ssid = doc["ssid"] | "";
        String pass = doc["password"] | "";

        if (ssid.length() == 0) {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Network SSID is required\"}");
            return;
        }

        Preferences p;
        p.begin("aura", false);
        p.putString("wifiSsid", ssid);
        p.putString("wifiPass", pass);
        p.putUChar("bootCount", 0);
        p.end();

        Serial.printf("[WiFi] Saved credentials for '%s'. Scheduling system restart...\n", ssid.c_str());
        request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Credentials saved. Reconnecting...\"}");

        xTaskCreate([](void*) {
            vTaskDelay(pdMS_TO_TICKS(1500));
            ESP.restart();
        }, "ota_reboot", 2048, NULL, 1, NULL);
    });

    // REST API - Factory Reset Wi-Fi into SoftAP Setup Mode
    server.on("/api/wifi/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        Preferences p;
        p.begin("aura", false);
        p.putString("wifiSsid", "");
        p.putString("wifiPass", "");
        p.putUChar("bootCount", 0);
        p.end();

        Serial.println("[WiFi] Credentials reset by user. Rebooting into Setup Hotspot...");
        request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Wi-Fi credentials reset. Rebooting into Setup Mode...\"}");

        xTaskCreate([](void*) {
            vTaskDelay(pdMS_TO_TICKS(1500));
            ESP.restart();
        }, "ota_reboot", 2048, NULL, 1, NULL);
    });

    // 404 Handler with automatic captive redirect and rescue page
    server.onNotFound([this](AsyncWebServerRequest *request) {
        if (captiveActive) {
            request->redirect("http://192.168.4.1/setup");
            return;
        }
        if (!LittleFS.exists("/index.html")) {
            String rescue = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><title>The Aura Studio Setup</title><style>body{background:#07090e;color:#f8fafc;font-family:-apple-system,sans-serif;padding:40px;text-align:center}h1{color:#00f2fe;font-size:24px}p{color:#94a3b8;font-size:16px;max-width:500px;margin:12px auto}.box{background:rgba(255,255,255,0.05);border:1px solid rgba(255,255,255,0.1);border-radius:12px;padding:24px;display:inline-block;margin-top:20px}code{background:rgba(0,242,254,0.15);color:#00f2fe;padding:4px 8px;border-radius:6px;font-family:monospace}</style></head><body><h1>The Aura Studio // ESP32 Smart Light Hub</h1><p style='color:#a855f7;font-weight:600;letter-spacing:1px;font-size:12px'>DESIGNED BY SANDY</p><div class='box'><p>Firmware is running and online! However, dashboard files are not found on flash memory.</p><p>In PlatformIO, run <code>Upload Filesystem Image</code> to flash the web dashboard to LittleFS.</p></div></body></html>";
            request->send(200, "text/html", rescue);
        } else {
            request->send(404, "text/plain", "The Aura Studio Hub: File Not Found");
        }
    });
}

void WebServerManager::cleanupClients() {
    ws.cleanupClients();
}

String WebServerManager::serializeState() {
    HubState localState;
    if (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        localState = state;
        xSemaphoreGive(stateMutex);
    } else {
        localState = state;
    }

    JsonDocument doc;
    doc["type"] = "state";
    JsonObject payload = doc["payload"].to<JsonObject>();

    payload["isMasterOn"] = localState.isMasterOn;
    payload["masterBrightness"] = localState.masterBrightness;
    payload["contrastRatio"] = localState.contrastRatio;
    payload["syncCorners"] = localState.syncCorners;
    payload["activePreset"] = localState.activePreset;
    payload["firmwareVersion"] = localState.firmwareVersion;
    payload["useGammaCorrection"] = localState.useGammaCorrection;
    payload["onOffAnimation"] = localState.onOffAnimation;

    JsonObject net = payload["network"].to<JsonObject>();
    net["hubIp"] = WiFi.localIP().toString();
    net["wizLeftIp"] = localState.wizLeftIp;
    net["wizLeftMac"] = localState.wizLeftMac;
    net["wizRightIp"] = localState.wizRightIp;
    net["wizRightMac"] = localState.wizRightMac;
    net["wifiSsid"] = (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : localState.wifiSsid;
    net["wifiRssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : localState.wifiRssi;
    net["isSetupMode"] = captiveActive || localState.isSetupMode;

    JsonObject dev = payload["devices"].to<JsonObject>();

    // Desk
    JsonObject deskObj = dev["desk"].to<JsonObject>();
    deskObj["power"] = localState.desk.power;
    deskObj["brightness"] = localState.desk.brightness;
    char hexDesk[8];
    snprintf(hexDesk, sizeof(hexDesk), "#%02x%02x%02x", localState.desk.r, localState.desk.g, localState.desk.b);
    deskObj["color"] = hexDesk;
    deskObj["effect"] = localState.desk.effect;
    deskObj["speed"] = localState.desk.speed;
    deskObj["intensity"] = localState.desk.intensity;
    deskObj["turbulence"] = localState.desk.intensity;
    if (localState.desk.hasGradient) {
        JsonArray stops = deskObj["gradientStops"].to<JsonArray>();
        for (int i = 0; i < 4; i++) {
            char hexStop[8];
            snprintf(hexStop, sizeof(hexStop), "#%02x%02x%02x",
                     localState.desk.stops[i].r, localState.desk.stops[i].g, localState.desk.stops[i].b);
            stops.add(hexStop);
        }
    }

    // WiZ Left
    JsonObject wlObj = dev["wizLeft"].to<JsonObject>();
    wlObj["power"] = localState.wizLeft.power;
    wlObj["brightness"] = localState.wizLeft.brightness;
    wlObj["mode"] = localState.wizLeft.mode;
    char hexWl[8];
    snprintf(hexWl, sizeof(hexWl), "#%02x%02x%02x", localState.wizLeft.r, localState.wizLeft.g, localState.wizLeft.b);
    wlObj["color"] = hexWl;
    wlObj["kelvin"] = localState.wizLeft.kelvin;

    // WiZ Right
    JsonObject wrObj = dev["wizRight"].to<JsonObject>();
    wrObj["power"] = localState.wizRight.power;
    wrObj["brightness"] = localState.wizRight.brightness;
    wrObj["mode"] = localState.wizRight.mode;
    char hexWr[8];
    snprintf(hexWr, sizeof(hexWr), "#%02x%02x%02x", localState.wizRight.r, localState.wizRight.g, localState.wizRight.b);
    wrObj["color"] = hexWr;
    wrObj["kelvin"] = localState.wizRight.kelvin;

    String output;
    serializeJson(doc, output);
    return output;
}

void WebServerManager::broadcastState() {
    ws.textAll(serializeState());
}

void WebServerManager::broadcastDiscoveredDevices() {
    JsonDocument doc;
    doc["type"] = "wizDiscovery";
    JsonArray devices = doc["devices"].to<JsonArray>();

    for (const auto& b : wiz.getDiscoveredBulbs()) {
        JsonObject obj = devices.add<JsonObject>();
        obj["ip"] = b.ip;
        obj["mac"] = b.mac;
        obj["model"] = b.model;
        obj["rssi"] = b.rssi;
    }

    String output;
    serializeJson(doc, output);
    ws.textAll(output);
}

String WebServerManager::getSetupPortalHtml() {
    return String(R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>The Aura Studio // Wi-Fi Setup</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background: #07090e;
      color: #f8fafc;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      display: flex;
      align-items: center;
      justify-content: center;
      min-height: 100vh;
      padding: 24px;
    }
    .card {
      background: rgba(255, 255, 255, 0.03);
      border: 1px solid rgba(255, 255, 255, 0.08);
      backdrop-filter: blur(24px);
      -webkit-backdrop-filter: blur(24px);
      border-radius: 24px;
      padding: 36px 28px;
      width: 100%;
      max-width: 440px;
      box-shadow: 0 30px 60px -12px rgba(0, 0, 0, 0.7);
    }
    .badge-pill {
      display: inline-block;
      padding: 5px 14px;
      background: rgba(0, 242, 254, 0.12);
      border: 1px solid rgba(0, 242, 254, 0.3);
      border-radius: 9999px;
      font-size: 11px;
      font-weight: 700;
      letter-spacing: 1.5px;
      color: #00f2fe;
      text-transform: uppercase;
      margin-bottom: 16px;
    }
    h1 {
      font-size: 24px;
      font-weight: 700;
      color: #ffffff;
      letter-spacing: -0.5px;
      margin-bottom: 6px;
    }
    .subtitle {
      font-size: 13px;
      color: #94a3b8;
      line-height: 1.5;
      margin-bottom: 24px;
    }
    .field-group {
      margin-bottom: 18px;
      text-align: left;
    }
    label {
      display: block;
      font-size: 12px;
      font-weight: 600;
      color: #cbd5e1;
      text-transform: uppercase;
      letter-spacing: 0.8px;
      margin-bottom: 8px;
    }
    .select-wrap, .input-wrap {
      position: relative;
    }
    select, input[type="text"], input[type="password"] {
      width: 100%;
      padding: 13px 16px;
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid rgba(255, 255, 255, 0.12);
      border-radius: 12px;
      color: #ffffff;
      font-size: 14px;
      outline: none;
      transition: all 0.2s ease;
    }
    select:focus, input:focus {
      border-color: #00f2fe;
      background: rgba(0, 242, 254, 0.05);
      box-shadow: 0 0 0 3px rgba(0, 242, 254, 0.15);
    }
    select option {
      background: #0f172a;
      color: #ffffff;
    }
    .scan-btn {
      width: 100%;
      padding: 11px;
      background: rgba(255, 255, 255, 0.06);
      border: 1px solid rgba(255, 255, 255, 0.12);
      border-radius: 10px;
      color: #00f2fe;
      font-size: 13px;
      font-weight: 600;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      margin-bottom: 12px;
      transition: all 0.2s;
    }
    .scan-btn:hover {
      background: rgba(0, 242, 254, 0.1);
      border-color: #00f2fe;
    }
    .toggle-pass {
      position: absolute;
      right: 12px;
      top: 50%;
      transform: translateY(-50%);
      background: transparent;
      border: none;
      color: #94a3b8;
      cursor: pointer;
      font-size: 13px;
      padding: 4px;
    }
    .btn-submit {
      width: 100%;
      padding: 14px;
      margin-top: 10px;
      background: linear-gradient(135deg, #00f2fe 0%, #a855f7 100%);
      border: none;
      border-radius: 14px;
      color: #07090e;
      font-size: 14px;
      font-weight: 700;
      letter-spacing: 0.5px;
      cursor: pointer;
      transition: opacity 0.2s, transform 0.1s;
    }
    .btn-submit:hover { opacity: 0.95; }
    .btn-submit:active { transform: scale(0.99); }
    .btn-submit:disabled { opacity: 0.4; cursor: not-allowed; }
    .status-msg {
      margin-top: 16px;
      font-size: 13px;
      padding: 10px;
      border-radius: 10px;
      display: none;
      line-height: 1.4;
    }
    .status-msg.info { display: block; background: rgba(0, 242, 254, 0.1); color: #00f2fe; border: 1px solid rgba(0, 242, 254, 0.2); }
    .status-msg.success { display: block; background: rgba(34, 197, 94, 0.1); color: #4ade80; border: 1px solid rgba(34, 197, 94, 0.2); }
    .status-msg.error { display: block; background: rgba(239, 68, 68, 0.1); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.2); }
    .sig { margin-top: 24px; font-size: 11px; letter-spacing: 1px; color: #64748b; text-transform: uppercase; }
  </style>
</head>
<body>
  <div class="card">
    <div class="badge-pill">Autonomous Setup Portal</div>
    <h1>The Aura Studio</h1>
    <p class="subtitle">Connect your smart light hub to your home Wi-Fi network.</p>

    <button type="button" class="scan-btn" id="scanBtn" onclick="scanWifi()">
      <span>⚡ Scan 2.4GHz Networks</span>
    </button>

    <div class="field-group">
      <label for="ssidSelect">Discovered Networks</label>
      <div class="select-wrap">
        <select id="ssidSelect" onchange="onSelectNetwork(this.value)">
          <option value="">Select a network or enter below...</option>
        </select>
      </div>
    </div>

    <div class="field-group">
      <label for="ssidInput">Network Name (SSID)</label>
      <input type="text" id="ssidInput" placeholder="Enter Wi-Fi SSID" autocomplete="off" spellcheck="false" required>
    </div>

    <div class="field-group">
      <label for="passInput">Wi-Fi Password</label>
      <div class="input-wrap">
        <input type="password" id="passInput" placeholder="Enter network password" autocomplete="current-password">
        <button type="button" class="toggle-pass" onclick="togglePassVisibility()">👁</button>
      </div>
    </div>

    <button type="button" class="btn-submit" id="saveBtn" onclick="saveCredentials()">Save & Connect to Wi-Fi</button>

    <div id="statusBox" class="status-msg"></div>

    <div class="sig">Designed by Sandy // Dual-Target Architecture</div>
  </div>

  <script>
    function togglePassVisibility() {
      const p = document.getElementById('passInput');
      p.type = (p.type === 'password') ? 'text' : 'password';
    }

    function onSelectNetwork(val) {
      if (val) {
        document.getElementById('ssidInput').value = val;
      }
    }

    function showStatus(msg, type) {
      const b = document.getElementById('statusBox');
      b.className = 'status-msg ' + type;
      b.innerText = msg;
    }

    function scanWifi() {
      const btn = document.getElementById('scanBtn');
      const sel = document.getElementById('ssidSelect');
      btn.disabled = true;
      btn.innerText = 'Scanning airwaves...';
      showStatus('Scanning 2.4GHz channels for nearby networks...', 'info');

      fetch('/api/wifi/scan')
        .then(r => r.json())
        .then(data => {
          btn.disabled = false;
          btn.innerText = '⚡ Scan 2.4GHz Networks';
          sel.innerHTML = '<option value="">Select a network or enter below...</option>';
          if (data && data.networks && data.networks.length > 0) {
            data.networks.forEach(n => {
              const opt = document.createElement('option');
              opt.value = n.ssid;
              const lock = n.secure ? '🔒 ' : '';
              opt.textContent = `${lock}${n.ssid} (${n.rssi} dBm)`;
              sel.appendChild(opt);
            });
            showStatus(`Discovered ${data.networks.length} networks nearby.`, 'info');
          } else {
            showStatus('No networks found. You can enter your SSID manually below.', 'error');
          }
        })
        .catch(err => {
          btn.disabled = false;
          btn.innerText = '⚡ Scan 2.4GHz Networks';
          showStatus('Scan error. Please enter SSID manually.', 'error');
        });
    }

    function saveCredentials() {
      const ssid = document.getElementById('ssidInput').value.trim();
      const pass = document.getElementById('passInput').value;
      const btn = document.getElementById('saveBtn');

      if (!ssid) {
        showStatus('Please specify a network SSID.', 'error');
        return;
      }

      btn.disabled = true;
      showStatus('Saving credentials & connecting to Wi-Fi...', 'info');

      fetch('/api/wifi/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid: ssid, password: pass })
      })
      .then(r => r.json())
      .then(res => {
        showStatus('Credentials saved! Hub is rebooting and joining ' + ssid + '... Connect your phone to ' + ssid + ' and open http://aura-hub.local', 'success');
      })
      .catch(err => {
        showStatus('Saved! Hub is restarting to connect. Please reconnect your device to ' + ssid, 'success');
      });
    }

    // Auto-scan on captive portal load
    window.addEventListener('DOMContentLoaded', () => {
      scanWifi();
    });
  </script>
</body>
</html>)rawliteral");
}
