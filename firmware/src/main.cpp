#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
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
    Serial.println("==========================================");

    // Initialize Mutex
    stateMutex = xSemaphoreCreateMutex();

    // 1. Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed. Formatting...");
    } else {
        Serial.println("[FS] LittleFS Mounted Successfully.");
    }

    // 2. Initialize Hardware LED Strip
    ledCtrl.initHardware();

    // 3. Connect to Wi-Fi in Background (Non-blocking negotiation on Core 0)
    Serial.printf("[WiFi] Initiating connection to %s in background...\n", DEFAULT_WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);

    // 4. Play Apple Siri HomePod Physics Fluid Wave (2500ms at 60 FPS on Core 1)
    // While the fluid wave is actively playing, Wi-Fi connects concurrently!
    ledCtrl.playSiriBootupAnimation(2500);

    // 5. Finalize Wi-Fi Connection State
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
        delay(150);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected successfully!");
        Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
        WiFi.setSleep(false); // Disable Wi-Fi power-save sleep to eliminate timing latency spikes

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
        Serial.println("\n[WiFi] Could not connect to AP. Starting SoftAP Fallback...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("AURA-SMART-HUB", "aura1234");
        Serial.printf("[WiFi] SoftAP started! IP: %s (Connect and open 192.168.4.1)\n", WiFi.softAPIP().toString().c_str());
    }

    // 4. Initialize WiZ Controller (UDP) with HubState binding
    wizCtrl.begin(&hubState);

    // 5. Initialize Web Server and WebSockets
    webManager.begin();

    // Trigger background UDP discovery on boot to auto-rebind known bulb MACs
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiZ] Triggering boot-time UDP discovery broadcast for auto-rebind...");
        wizCtrl.broadcastDiscovery();
    }

    // 6. Spawn Core 1 FreeRTOS Task for 60 FPS WS2812B Rendering
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
    // Handle background OTA requests
    ArduinoOTA.handle();

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
