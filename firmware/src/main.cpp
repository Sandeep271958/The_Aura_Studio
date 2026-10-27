#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include "Config.h"
#include "WizController.h"
#include "LedDriver.h"
#include "WebServerManager.h"

// Global instances
HubState hubState;
WizController wizCtrl;
LedDriver ledCtrl;
WebServerManager webManager(hubState, wizCtrl);

// Task handle for LED rendering on Core 1
TaskHandle_t LedTaskHandle = NULL;

void LedTask(void *pvParameters) {
    Serial.printf("[Core 1] LED Task pinned to core %d\n", xPortGetCoreID());
    for (;;) {
        ledCtrl.update(hubState.desk, hubState.isMasterOn, hubState.masterBrightness, hubState.contrastRatio);
        vTaskDelay(pdMS_TO_TICKS(16)); // Target 60 FPS
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   THE AURA STUDIO // ESP32 SMART LIGHT HUB");
    Serial.println("   Designed by Sandy                        ");
    Serial.println("==========================================");

    // 1. Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed. Formatting...");
    } else {
        Serial.println("[FS] LittleFS Mounted Successfully.");
    }

    // 2. Initialize Hardware LED Strip
    ledCtrl.begin();

    // 3. Connect to Wi-Fi
    Serial.printf("[WiFi] Connecting to %s", DEFAULT_WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected successfully!");
        Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());

        // Setup mDNS (http://aura-hub.local)
        if (MDNS.begin(HOSTNAME)) {
            Serial.printf("[mDNS] Responder started at http://%s.local\n", HOSTNAME);
            MDNS.addService("http", "tcp", 80);
        }
    } else {
        Serial.println("\n[WiFi] Could not connect to AP. Starting SoftAP Fallback...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("AURA-SMART-HUB", "aura1234");
        Serial.printf("[WiFi] SoftAP started! IP: %s (Connect and open 192.168.4.1)\n", WiFi.softAPIP().toString().c_str());
    }

    // 4. Initialize WiZ Controller (UDP)
    wizCtrl.begin();

    // 5. Initialize Web Server and WebSockets
    webManager.begin();

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
    // Process WiZ discovery UDP packets
    wizCtrl.updateDiscovery();

    // Periodic discovery broadcast check if devices found
    static uint32_t lastDevCheck = 0;
    if (millis() - lastDevCheck > 1000) {
        lastDevCheck = millis();
        if (wizCtrl.getDiscoveredBulbs().size() > 0) {
            webManager.broadcastDiscoveredDevices();
        }
    }

    // Clean up disconnected WebSocket clients periodically
    webManager.cleanupClients();

    delay(20);
}
