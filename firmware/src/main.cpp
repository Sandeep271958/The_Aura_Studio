#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include "Config.h"
#include "WizController.h"
#include "LedDriver.h"
#include "WebServerManager.h"

// Global instances
HubState hubState;
WizController wizCtrl;
LedDriver ledCtrl;
WebServerManager webManager(hubState, wizCtrl);

// Persistent active network credentials
static String activeSsid = "";
static String activePass = "";

// FreeRTOS Mutex & Task handle for LED rendering on Core 1
SemaphoreHandle_t stateMutex = NULL;
TaskHandle_t LedTaskHandle = NULL;

void LedTask(void *pvParameters) {
    Serial.printf("[Core 1] LED Task pinned to core %d\n", xPortGetCoreID());
    DeviceDesk localDesk;
    bool localMasterOn = true;
    uint8_t localMasterBrightness = 80;
    int8_t localContrastRatio = 0;
    bool localUseGammaCorrection = true;
    String localOnOffAnimation = "gaussian-ripple";

    for (;;) {
        if (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            localDesk = hubState.desk;
            localMasterOn = hubState.isMasterOn;
            localMasterBrightness = hubState.masterBrightness;
            localContrastRatio = hubState.contrastRatio;
            localUseGammaCorrection = hubState.useGammaCorrection;
            localOnOffAnimation = hubState.onOffAnimation;
            xSemaphoreGive(stateMutex);
        }
        ledCtrl.update(localDesk, localMasterOn, localMasterBrightness, localContrastRatio, localUseGammaCorrection, localOnOffAnimation);
        vTaskDelay(pdMS_TO_TICKS(22)); // Fluid 45 FPS cinema-smooth animation loop (relieves bus contention & voltage sag)
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   THE AURA STUDIO // ESP32 SMART LIGHT HUB");
    Serial.println("   Designed by Sandy                        ");
    Serial.println("   Industrial Robustness & Dual-OTA         ");
    Serial.println("==========================================");

    // Hardware Reset Reason Audit
    esp_reset_reason_t rstReason = esp_reset_reason();
    Serial.printf("[Boot] Hardware Reset Reason: %d\n", (int)rstReason);
    if (rstReason == ESP_RST_BROWNOUT) {
        Serial.println("[Boot] WARNING: Hardware Brownout Detected! Enforcing conservative power profile.");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, 950);
    }

    // Initialize Mutex
    stateMutex = xSemaphoreCreateMutex();

    // 1. Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed. Formatting...");
    } else {
        Serial.println("[FS] LittleFS Mounted Successfully.");
    }

    // 2. Load Network Credentials safely from NVS (Never wipe credentials on rapid power cycles!)
    Preferences bootPrefs;
    bootPrefs.begin("aura", false);
    activeSsid = bootPrefs.getString("wifiSsid", "");
    activePass = bootPrefs.getString("wifiPass", "");
    bootPrefs.end();

    if (activeSsid.length() == 0) {
        activeSsid = DEFAULT_WIFI_SSID;
        activePass = DEFAULT_WIFI_PASSWORD;
    }

    // 2.5 Load Full Persistent State from NVS (Ensures Hub returns to exact previous state)
    webManager.loadPersistentState();

    // 3. Initialize Hardware LED Strip
    ledCtrl.initHardware();

    // Check if system was switched off before power down
    bool systemWasSwitchedOff = (!hubState.isMasterOn || !hubState.desk.power);

    // 4. Wi-Fi Negotiation or SoftAP Setup
    bool wifiConnected = false;
    if (activeSsid.length() > 0) {
        Serial.printf("[WiFi] Initiating connection to '%s'...\n", activeSsid.c_str());
        WiFi.setTxPower(WIFI_POWER_17dBm); // Prevent high-current RF inrush spikes
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFi.begin(activeSsid.c_str(), activePass.c_str());

        if (systemWasSwitchedOff) {
            Serial.println("[Boot] Restored previous power state: SWITCHED OFF. Keeping hardware LEDs dark.");
            FastLED.clear(true);
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 35) {
                delay(150);
                Serial.print(".");
                attempts++;
            }
            wifiConnected = (WiFi.status() == WL_CONNECTED);
        } else {
            // Play Apple Siri HomePod Physics Fluid Wave (2500ms at 60 FPS on Core 1)
            // Wi-Fi handshake negotiates concurrently during the fluid wave!
            ledCtrl.playSiriBootupAnimation(2500);

            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 30) {
                delay(150);
                Serial.print(".");
                attempts++;
            }
            wifiConnected = (WiFi.status() == WL_CONNECTED);
        }
    }

    if (wifiConnected) {
        Serial.println("\n[WiFi] Connected successfully!");
        Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
        WiFi.setSleep(false); // Disable Wi-Fi power-save sleep to eliminate latency spikes

        hubState.wifiSsid = activeSsid;
        hubState.wifiRssi = WiFi.RSSI();
        hubState.isSetupMode = false;

        // Setup mDNS (http://aura-hub.local)
        if (MDNS.begin(HOSTNAME)) {
            Serial.printf("[mDNS] Responder started at http://%s.local\n", HOSTNAME);
            MDNS.addService("http", "tcp", 80);
        }

        // Initialize ArduinoOTA for wireless PlatformIO flashing
        ArduinoOTA.setHostname(HOSTNAME);
        ArduinoOTA.onStart([]() {
            Serial.println("[OTA] Wireless flash in progress...");
        });
        ArduinoOTA.onEnd([]() {
            Serial.println("\n[OTA] Wireless flash complete! Rebooting...");
        });
        ArduinoOTA.begin();
        Serial.println("[OTA] ArduinoOTA service initialized and listening.");
    } else {
        Serial.println("\n[WiFi] Station connection timed out (Router may be booting). Entering Autonomous AP+STA Mode...");
        hubState.isSetupMode = true;
        hubState.wifiSsid = activeSsid; // Preserve activeSsid for background auto-reconnect
        hubState.wifiRssi = 0;

        // Start AP+STA mode to broadcast setup hotspot while background station reconnection probes continue
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS);
        Serial.printf("[WiFi] SoftAP started! SSID: '%s' | IP: %s\n", SOFTAP_SSID, WiFi.softAPIP().toString().c_str());

        // Start Captive Portal DNS on port 53 (redirects all domains to 192.168.4.1)
        webManager.enableCaptivePortal();

        // Amber breathing beacon signals Hub is in Setup Mode awaiting Wi-Fi configuration
        hubState.isMasterOn = true;
        hubState.desk.power = true;
        hubState.desk.effect = "breathe";
        hubState.desk.r = 251;
        hubState.desk.g = 146;
        hubState.desk.b = 60;
        hubState.desk.brightness = 90;
        hubState.desk.speed = 45;
    }

    // 5. Initialize WiZ Controller (UDP) with HubState binding
    wizCtrl.begin(&hubState);

    // 6. Initialize Web Server, Captive Portal, and WebSockets
    webManager.begin();

    // Trigger background UDP discovery on boot if connected to local LAN
    if (wifiConnected) {
        Serial.println("[WiZ] Triggering boot-time UDP discovery broadcast & initial state sync...");
        wizCtrl.broadcastDiscovery();
        wizCtrl.syncAllBulbs();
    }

    // 7. Spawn Core 1 FreeRTOS Task for 60 FPS WS2812B Rendering (8192 bytes stack headroom)
    xTaskCreatePinnedToCore(
        LedTask,
        "LedTask",
        8192,
        NULL,
        2,
        &LedTaskHandle,
        1
    );

    Serial.println("[System] The Aura Studio Hub Ready & Operational.\n");
}

void loop() {
    // Process Captive Portal DNS queries if in setup mode
    webManager.processDns();

    // Handle background OTA requests
    ArduinoOTA.handle();

    // Autonomous Background Wi-Fi Reconnection Engine (Self-heals after router power outage)
    static uint32_t lastStaRetryMs = 0;
    if (hubState.isSetupMode && activeSsid.length() > 0 && (millis() - lastStaRetryMs >= 5000)) {
        lastStaRetryMs = millis();
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[WiFi Auto-Recovery] Home network detected and connected in background!");
            Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());

            hubState.isSetupMode = false;
            hubState.wifiSsid = activeSsid;
            hubState.wifiRssi = WiFi.RSSI();

            // Stop captive portal
            webManager.disableCaptivePortal();

            // Switch to pure station mode
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            WiFi.setSleep(false);

            // Start mDNS and OTA
            if (MDNS.begin(HOSTNAME)) {
                Serial.printf("[mDNS] Responder started at http://%s.local\n", HOSTNAME);
                MDNS.addService("http", "tcp", 80);
            }
            ArduinoOTA.setHostname(HOSTNAME);
            ArduinoOTA.begin();

            // Sync WiZ bulbs now that local LAN is reachable
            wizCtrl.broadcastDiscovery();
            wizCtrl.syncAllBulbs();

            // Broadcast updated state to all connected clients
            webManager.broadcastState();

            // Restore normal power limit
            FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
        } else {
            Serial.printf("[WiFi Auto-Recovery] Probing background reconnect to '%s'...\n", activeSsid.c_str());
            WiFi.begin(activeSsid.c_str(), activePass.c_str());
        }
    }

    // Periodically update Wi-Fi RSSI signal telemetry
    static uint32_t lastRssiCheck = 0;
    if (millis() - lastRssiCheck > 5000) {
        lastRssiCheck = millis();
        if (WiFi.status() == WL_CONNECTED) {
            hubState.wifiRssi = WiFi.RSSI();
        }
    }

    // Process WiZ discovery UDP packets
    wizCtrl.updateDiscovery();

    // Active health heartbeat & periodic status poll to detect newly powered bulbs
    wizCtrl.updateHeartbeat();

    // Broadcast discovered bulbs only when discovery count changes
    static size_t lastReportedBulbCount = 0;
    size_t currentBulbCount = wizCtrl.getDiscoveredBulbs().size();
    if (currentBulbCount != lastReportedBulbCount) {
        lastReportedBulbCount = currentBulbCount;
        webManager.broadcastDiscoveredDevices();
    }

    // Broadcast updated state if bulb IP auto-rebound via DHCP
    if (wizCtrl.checkAndClearRebound()) {
        Serial.println("[WiZ] Bulb IP auto-rebound complete. Broadcasting updated state to dashboard...");
        webManager.broadcastState();
    }

    // Clean up disconnected WebSocket clients periodically
    webManager.cleanupClients();

    // Commit debounced state changes to NVS if dirty
    webManager.updatePersistence();

    delay(20);
}
