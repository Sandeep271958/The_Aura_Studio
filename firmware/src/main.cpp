#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
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
    esp_task_wdt_reset();
    delay(100);
    esp_task_wdt_reset();

    Serial.println("\n==========================================");
    Serial.println("   THE AURA STUDIO // ESP32 SMART LIGHT HUB");
    Serial.println("   Designed by Sandy                        ");
    Serial.println("   Industrial Robustness & Staged Boot      ");
    Serial.println("==========================================");

    // Hardware Reset Reason Audit
    esp_reset_reason_t rstReason = esp_reset_reason();
    Serial.printf("[Boot] Hardware Reset Reason: %d\n", (int)rstReason);
    if (rstReason == ESP_RST_BROWNOUT) {
        Serial.println("[Boot] WARNING: Hardware Brownout Detected! Enforcing conservative power profile.");
    }

    // -------------------------------------------------------------
    // STAGE 1: Pure Isolated Hardware Alive Wave (Zero RF Inrush)
    // -------------------------------------------------------------
    // Keep Wi-Fi radio completely unpowered during LED boot animation to eliminate power brownouts
    WiFi.mode(WIFI_OFF);
    ledCtrl.initHardware();

    // ALWAYS play Apple Siri HomePod Physics Fluid Wave on boot!
    // Provides immediate visual confirmation that the ESP32 is powered and alive.
    // Clamped to 450mA (<220mA total current draw) with watchdog reset on every frame.
    ledCtrl.playSiriBootupAnimation(2500);
    esp_task_wdt_reset();

    // -------------------------------------------------------------
    // STAGE 2: Core Subsystems & Web Server Initialization
    // -------------------------------------------------------------
    // Initialize Mutex
    stateMutex = xSemaphoreCreateMutex();

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed. Formatting...");
    } else {
        Serial.println("[FS] LittleFS Mounted Successfully.");
    }
    esp_task_wdt_reset();

    // Load Network Credentials safely from NVS
    Preferences bootPrefs;
    bootPrefs.begin("aura", false);
    activeSsid = bootPrefs.getString("wifiSsid", "");
    activePass = bootPrefs.getString("wifiPass", "");
    bootPrefs.end();

    if (activeSsid.length() == 0) {
        activeSsid = DEFAULT_WIFI_SSID;
        activePass = DEFAULT_WIFI_PASSWORD;
    }

    // Load Full Persistent State from NVS (Ensures Hub returns to exact previous state_t0)
    webManager.loadPersistentState();
    esp_task_wdt_reset();

    // Check if system was switched off before power down
    bool systemWasSwitchedOff = (!hubState.isMasterOn || !hubState.desk.power);
    Serial.printf("[Boot] Restored previous power state: %s (Master: %s | Desk: %s)\n",
                  systemWasSwitchedOff ? "OFF" : "ON",
                  hubState.isMasterOn ? "ON" : "OFF",
                  hubState.desk.power ? "ON" : "OFF");

    // Initialize WiZ Controller (UDP) with HubState binding
    wizCtrl.begin(&hubState);

    // Initialize Web Server, Captive Portal, and WebSockets (Port 80 ready immediately)
    webManager.begin();
    esp_task_wdt_reset();

    // Spawn Core 1 FreeRTOS Task for 45-60 FPS WS2812B Rendering (8192 bytes stack headroom)
    xTaskCreatePinnedToCore(
        LedTask,
        "LedTask",
        8192,
        NULL,
        2,
        &LedTaskHandle,
        1
    );

    // -------------------------------------------------------------
    // STAGE 3: Staged Wi-Fi Bring-Up with Instant SoftAP Resilience
    // -------------------------------------------------------------
    bool wifiConnected = false;
    if (activeSsid.length() > 0) {
        Serial.printf("[WiFi] Initiating connection to '%s' (SoftAP '%s' active for immediate dashboard access)...\n",
                      activeSsid.c_str(), SOFTAP_SSID);

        // Start AP+STA mode immediately so the setup dashboard at 192.168.4.1 is available within seconds,
        // even if the home router is offline or booting after a power cut.
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS);
        webManager.enableCaptivePortal();

        WiFi.setTxPower(WIFI_POWER_17dBm); // Prevent high-current RF inrush spikes
        WiFi.setAutoReconnect(true);
        WiFi.begin(activeSsid.c_str(), activePass.c_str());

        // Non-blocking connection probe with watchdog feed (max 15 attempts * 150ms = ~2.25s)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 15) {
            esp_task_wdt_reset();
            delay(150);
            Serial.print(".");
            attempts++;
        }
        wifiConnected = (WiFi.status() == WL_CONNECTED);
    } else {
        // Fresh device without configured credentials
        WiFi.mode(WIFI_AP);
        WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS);
        webManager.enableCaptivePortal();
    }
    esp_task_wdt_reset();

    if (wifiConnected) {
        Serial.println("\n[WiFi] Connected successfully to home network!");
        Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());

        // Station connected: shut down setup hotspot and captive portal
        webManager.disableCaptivePortal();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
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

        // Trigger boot-time UDP discovery broadcast & initial state sync for WiZ bulbs
        Serial.println("[WiZ] Triggering boot-time UDP discovery broadcast & initial state sync...");
        wizCtrl.broadcastDiscovery();
        wizCtrl.syncAllBulbs();

        // Restore full operational power budget
        FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
    } else {
        Serial.printf("\n[WiFi] Station not connected yet (Router may be starting). Autonomous SoftAP active at %s!\n",
                      WiFi.softAPIP().toString().c_str());
        hubState.isSetupMode = true;
        hubState.wifiSsid = activeSsid;
        hubState.wifiRssi = 0;

        // Amber breathing beacon is only activated if no network was ever configured (fresh setup)
        // If the system has saved Wi-Fi credentials, PRESERVE restored state_t0!
        if (activeSsid.length() == 0 || activeSsid == DEFAULT_WIFI_SSID) {
            hubState.isMasterOn = true;
            hubState.desk.power = true;
            hubState.desk.effect = "breathe";
            hubState.desk.r = 251;
            hubState.desk.g = 146;
            hubState.desk.b = 60;
            hubState.desk.brightness = 90;
            hubState.desk.speed = 45;
        } else {
            Serial.printf("[Boot] Preserving restored state_t0 (Power: %s) while waiting for router in background.\n",
                          systemWasSwitchedOff ? "OFF" : "ON");
        }
    }

    esp_task_wdt_reset();
    Serial.println("[System] The Aura Studio Hub Ready & Operational.\n");
}

void loop() {
    esp_task_wdt_reset();

    // Process Captive Portal DNS queries if in setup mode
    webManager.processDns();

    // Handle background OTA requests
    ArduinoOTA.handle();

    // Autonomous Background Wi-Fi Reconnection Engine (Self-heals after router power outage)
    static uint32_t lastStaRetryMs = 0;
    if (hubState.isSetupMode && activeSsid.length() > 0 && (millis() - lastStaRetryMs >= 15000)) {
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
            WiFi.disconnect();
            delay(50);
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

    esp_task_wdt_reset();
    delay(20);
}
