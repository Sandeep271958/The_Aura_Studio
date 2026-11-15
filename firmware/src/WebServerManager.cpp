#include "WebServerManager.h"
#include <LittleFS.h>
#include <Preferences.h>
#include <Update.h>

static Preferences prefs;

WebServerManager::WebServerManager(HubState& stateRef, WizController& wizRef)
    : server(80), ws("/ws"), state(stateRef), wiz(wizRef) {}

void WebServerManager::begin() {
    prefs.begin("aura", false);
    state.wizLeftIp = prefs.getString("wizLeftIp", DEFAULT_WIZ_LEFT_IP);
    state.wizLeftMac = prefs.getString("wizLeftMac", "");
    state.wizRightIp = prefs.getString("wizRightIp", DEFAULT_WIZ_RIGHT_IP);
    state.wizRightMac = prefs.getString("wizRightMac", "");
    state.useGammaCorrection = prefs.getBool("useGamma", true);

    Serial.printf("[NVS] Loaded WiZ Left: IP=%s MAC=%s\n", state.wizLeftIp.c_str(), state.wizLeftMac.c_str());
    Serial.printf("[NVS] Loaded WiZ Right: IP=%s MAC=%s\n", state.wizRightIp.c_str(), state.wizRightMac.c_str());
    Serial.printf("[NVS] Loaded Gamma Correction: %s\n", state.useGammaCorrection ? "ON" : "OFF");

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

            if (stateMutex != NULL && mutexTaken) xSemaphoreGive(stateMutex);

            // Sync WiZ bulbs over UDP outside mutex
            if (leftModeRgb) {
                wiz.sendPilotRGB(leftIp, leftPower, leftR, leftG, leftB, leftEffectiveDim);
            } else {
                wiz.sendPilotCCT(leftIp, leftPower, leftK, leftEffectiveDim);
            }

            if (rightModeRgb) {
                wiz.sendPilotRGB(rightIp, rightPower, rightR, rightG, rightB, rightEffectiveDim);
            } else {
                wiz.sendPilotCCT(rightIp, rightPower, rightK, rightEffectiveDim);
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
        String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>The Aura Studio // Wireless Firmware OTA</title><style>*{box-sizing:border-box;margin:0;padding:0}body{background:#07090e;color:#f8fafc;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px}.card{background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.08);backdrop-filter:blur(20px);border-radius:20px;padding:36px 32px;width:100%;max-width:440px;text-align:center;box-shadow:0 25px 50px -12px rgba(0,0,0,0.5)}.badge{display:inline-block;padding:4px 12px;background:rgba(0,242,254,0.1);border:1px solid rgba(0,242,254,0.25);border-radius:9999px;font-size:11px;font-weight:600;letter-spacing:1.5px;color:#00f2fe;text-transform:uppercase;margin-bottom:16px}h1{font-size:22px;font-weight:700;color:#ffffff;margin-bottom:8px;letter-spacing:-0.5px}p{font-size:13px;color:#94a3b8;line-height:1.5;margin-bottom:24px}.drop-zone{border:2px dashed rgba(255,255,255,0.15);border-radius:14px;padding:24px 16px;cursor:pointer;transition:all 0.2s;margin-bottom:20px;background:rgba(255,255,255,0.01)}.drop-zone:hover{border-color:#00f2fe;background:rgba(0,242,254,0.02)}.file-name{font-size:13px;color:#00f2fe;font-family:monospace;margin-top:8px;word-break:break-all}input[type=file]{display:none}.btn{width:100%;padding:14px;background:linear-gradient(135deg,#00f2fe,#a855f7);color:#07090e;border:none;border-radius:12px;font-size:14px;font-weight:700;letter-spacing:0.5px;cursor:pointer;transition:opacity 0.2s}.btn:hover{opacity:0.9}.btn:disabled{opacity:0.4;cursor:not-allowed}.progress-wrap{display:none;margin-top:20px}.progress-bar-bg{width:100%;height:8px;background:rgba(255,255,255,0.08);border-radius:999px;overflow:hidden}.progress-bar{height:100%;width:0%;background:linear-gradient(90deg,#00f2fe,#a855f7);border-radius:999px;transition:width 0.15s}.status-text{font-size:12px;color:#94a3b8;margin-top:10px;font-weight:500}.back-link{display:inline-block;margin-top:20px;font-size:12px;color:#64748b;text-decoration:none;transition:color 0.2s}.back-link:hover{color:#00f2fe}</style></head><body><div class='card'><div class='badge'>Wireless OTA</div><h1>Flash Firmware</h1><p>Select your compiled <code>firmware.bin</code> to update The Aura Studio wirelessly.</p><div class='drop-zone' onclick=\"document.getElementById('fileInput').click()\"><div style='font-size:24px;margin-bottom:6px'>⚡</div><div style='font-size:13px;font-weight:600'>Click to choose firmware.bin</div><div id='fileName' class='file-name'>No file selected</div></div><input type='file' id='fileInput' accept='.bin' onchange='onFileChosen(this)'><button id='uploadBtn' class='btn' disabled onclick='startUpload()'>Upload & Flash Over-The-Air</button><div id='progressWrap' class='progress-wrap'><div class='progress-bar-bg'><div id='progressBar' class='progress-bar'></div></div><div id='statusText' class='status-text'>Uploading firmware... 0%</div></div><a href='/' class='back-link'>← Return to Dashboard</a></div><script>let chosenFile=null;function onFileChosen(input){if(input.files&&input.files[0]){chosenFile=input.files[0];document.getElementById('fileName').innerText=chosenFile.name;document.getElementById('uploadBtn').disabled=false}}function startUpload(){if(!chosenFile)return;const btn=document.getElementById('uploadBtn');const wrap=document.getElementById('progressWrap');const bar=document.getElementById('progressBar');const txt=document.getElementById('statusText');btn.disabled=true;wrap.style.display='block';const formData=new FormData();formData.append('update',chosenFile);const xhr=new XMLHttpRequest();xhr.open('POST','/update',true);xhr.upload.onprogress=function(e){if(e.lengthComputable){const pct=Math.round((e.loaded/e.total)*100);bar.style.width=pct+'%';txt.innerText='Uploading firmware... '+pct+'%'}};xhr.onload=function(){if(xhr.status===200&&xhr.responseText.includes('OK')){bar.style.width='100%';txt.style.color='#00f2fe';let sec=6;txt.innerText='Flash Complete! Rebooting in '+sec+'s...';const timer=setInterval(()=>{sec--;if(sec>0){txt.innerText='Flash Complete! Rebooting in '+sec+'s...'}else{clearInterval(timer);txt.innerText='Reconnecting to Dashboard...';window.location.href='/'}},1000)}else{txt.style.color='#ef4444';txt.innerText='Flash Failed: '+(xhr.responseText||'Server error');btn.disabled=false}};xhr.onerror=function(){txt.style.color='#00f2fe';txt.innerText='Upload complete. Reconnecting in 5s...';setTimeout(()=>{window.location.href='/'},5000)};xhr.send(formData)}</script></body></html>";
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
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
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

    // 404 Handler with automatic rescue page if LittleFS image has not been uploaded yet
    server.onNotFound([](AsyncWebServerRequest *request) {
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

    JsonObject net = payload["network"].to<JsonObject>();
    net["hubIp"] = WiFi.localIP().toString();
    net["wizLeftIp"] = localState.wizLeftIp;
    net["wizLeftMac"] = localState.wizLeftMac;
    net["wizRightIp"] = localState.wizRightIp;
    net["wizRightMac"] = localState.wizRightMac;

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
