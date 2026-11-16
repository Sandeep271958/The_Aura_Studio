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
    Serial.println("   Autonomous Provisioning & Dual-OTA       ");
    Serial.println("==========================================");

    // Initialize Mutex
    stateMutex = xSemaphoreCreateMutex();

    // 1. Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed. Formatting...");
    } else {
        Serial.println("[FS] LittleFS Mounted Successfully.");
    }

    // 2. Hardware Power-Cycle Guard (3-Cycle Emergency Wi-Fi Reset)
    Preferences bootPrefs;
    bootPrefs.begin("aura", false);
    uint8_t bootCount = bootPrefs.getUChar("bootCount", 0);
    bootCount++;
    bootPrefs.putUChar("bootCount", bootCount);
    Serial.printf("[Boot] Consecutive boot count: %u\n", bootCount);

    bool forceSetupMode = false;
    if (bootCount >= 3) {
        Serial.println("[Reset] 3 Rapid Power Cycles detected! Resetting Wi-Fi credentials to Setup Mode...");
        bootPrefs.putString("wifiSsid", "");
        bootPrefs.putString("wifiPass", "");
        bootPrefs.putUChar("bootCount", 0);
        forceSetupMode = true;
    }

    String activeSsid = bootPrefs.getString("wifiSsid", "");
    String activePass = bootPrefs.getString("wifiPass", "");
    bootPrefs.end();

    if (activeSsid.length() == 0 && !forceSetupMode) {
        activeSsid = DEFAULT_WIFI_SSID;
        activePass = DEFAULT_WIFI_PASSWORD;
    }

    // 3. Initialize Hardware LED Strip
    ledCtrl.initHardware();

    // 4. Wi-Fi Negotiation or SoftAP Setup
    bool wifiConnected = false;
    if (!forceSetupMode && activeSsid.length() > 0) {
        Serial.printf("[WiFi] Initiating connection to '%s' in background...\n", activeSsid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFi.begin(activeSsid.c_str(), activePass.c_str());

        // Play Apple Siri HomePod Physics Fluid Wave (2500ms at 60 FPS on Core 1)
        // Wi-Fi handshake negotiates concurrently during the fluid wave!
        ledCtrl.playSiriBootupAnimation(2500);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 15) {
            delay(150);
            Serial.print(".");
            attempts++;
        }
        wifiConnected = (WiFi.status() == WL_CONNECTED);
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
        Serial.println("\n[WiFi] Entering Autonomous SoftAP Setup Mode...");
        hubState.isSetupMode = true;
        hubState.wifiSsid = "";
        hubState.wifiRssi = 0;

        // Start AP+STA mode to broadcast setup hotspot while scanning channels concurrently
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
        Serial.println("[WiZ] Triggering boot-time UDP discovery broadcast for auto-rebind...");
        wizCtrl.broadcastDiscovery();
    }

    // 7. Spawn Core 1 FreeRTOS Task for 60 FPS WS2812B Rendering
    xTaskCreatePinnedToCore(
        LedTask,
        "LedTask",
        4096,
        NULL,
        1,
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

    // Clear consecutive bootCount after 4 seconds of stable run
    static bool bootCountResetDone = false;
    if (!bootCountResetDone && millis() > 4000) {
        bootCountResetDone = true;
        Preferences p;
        p.begin("aura", false);
        p.putUChar("bootCount", 0);
        p.end();
        Serial.println("[Boot] Stable operation confirmed: bootCount cleared.");
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

    delay(20);
}
