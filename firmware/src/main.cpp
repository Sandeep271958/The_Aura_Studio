#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
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

// Arduino-ESP32 weak hook override: Defer auto-validation so the 20-second stability trial enforces rollback on boot crashes
extern "C" bool verifyRollbackLater(void) {
    return true;
}

// RTC Memory variable: survives warm reboots, panics, and software resets
RTC_DATA_ATTR static int rtcBootFailures = 0;

// FreeRTOS Mutex & Task handle for LED rendering on Core 1
SemaphoreHandle_t stateMutex = NULL;
TaskHandle_t LedTaskHandle = NULL;

void LedTask(void *pvParameters) {
    Serial.printf("[Core 1] LED Task pinned to core %d (Zero-Allocation Render Engine)\n", xPortGetCoreID());
    DeviceDesk localDesk;
    bool localMasterOn = true;
    uint8_t localMasterBrightness = 80;
    int8_t localContrastRatio = 0;
    bool localUseGammaCorrection = true;
    String localOnOffAnimation = "gaussian-ripple";
    uint32_t localVersion = 0xFFFFFFFF;

    for (;;) {
        // Zero-allocation & zero-mutex guard: only lock and copy when state version changes!
        if (hubState.deskVersion != localVersion) {
            if (stateMutex != NULL && xSemaphoreTake(stateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                localDesk = hubState.desk;
                localMasterOn = hubState.isMasterOn;
                localMasterBrightness = hubState.masterBrightness;
                localContrastRatio = hubState.contrastRatio;
                localUseGammaCorrection = hubState.useGammaCorrection;
                localOnOffAnimation = hubState.onOffAnimation;
                localVersion = hubState.deskVersion;
                xSemaphoreGive(stateMutex);
            }
        }
        ledCtrl.update(localDesk, localMasterOn, localMasterBrightness, localContrastRatio, localUseGammaCorrection, localOnOffAnimation);
        vTaskDelay(pdMS_TO_TICKS(22)); // Fluid 45 FPS cinema-smooth animation loop (relieves bus contention & voltage sag)
    }
}

void setup() {
    // Disable software-triggered brownout detector to prevent microsecond voltage dip resets on USB power
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(500); // 500ms power supply voltage stabilization delay

    Serial.println("\n==========================================");
    Serial.println("   THE AURA STUDIO // ESP32 SMART LIGHT HUB");
    Serial.println("   Designed by Sandy                        ");
    Serial.println("   Firmware: v03.00.000 (Fail-Safe Dual-OTA)");
    Serial.println("==========================================");

    // Hardware Reset Reason Audit
    esp_reset_reason_t rstReason = esp_reset_reason();
    Serial.printf("[Boot] Hardware Reset Reason: %d\n", (int)rstReason);
    if (rstReason == ESP_RST_BROWNOUT) {
        Serial.println("[Boot] WARNING: Hardware Brownout Detected! Enforcing conservative power profile.");
    }

    // -------------------------------------------------------------
    // DUAL-OTA PARTITION AUDIT & TIER-1 AUTO-ROLLBACK WATCHDOG
    // -------------------------------------------------------------
    const esp_partition_t *runningPart = esp_ota_get_running_partition();
    const esp_partition_t *otherPart = esp_ota_get_next_update_partition(NULL);
    if (runningPart != NULL) {
        hubState.activePartition = String(runningPart->label);
        Serial.printf("[OTA] Active Boot Partition: %s (Address: 0x%06lx)\n",
                      runningPart->label, (unsigned long)runningPart->address);

        esp_ota_img_states_t otaState;
        if (esp_ota_get_state_partition(runningPart, &otaState) == ESP_OK) {
            hubState.otaPendingVerify = (otaState == ESP_OTA_IMG_PENDING_VERIFY);
            Serial.printf("[OTA] Partition Verification State: %s\n",
                          hubState.otaPendingVerify ? "PENDING_VERIFY (20s Trial)" : "VALID");
        }
    }

    // Inspect hardware bootloader state & persistent trial metadata from NVS
    Preferences otaPrefs;
    otaPrefs.begin("aura_ota", false);
    bool trialActive = otaPrefs.getBool("trial_active", false);
    String fallbackPart = otaPrefs.getString("fallback_part", "");
    String failedPart = otaPrefs.getString("pending_part", "");

    bool bootloaderRolledBack = false;
    if (trialActive) {
        if (otherPart != NULL) {
            esp_ota_img_states_t otherState;
            if (esp_ota_get_state_partition(otherPart, &otherState) == ESP_OK) {
                if (otherState == ESP_OTA_IMG_ABORTED || otherState == ESP_OTA_IMG_INVALID) {
                    bootloaderRolledBack = true;
                    failedPart = String(otherPart->label);
                }
            }
        }

        if (fallbackPart.length() > 0 && hubState.activePartition == fallbackPart) {
            bootloaderRolledBack = true;
        }
    }

    if (bootloaderRolledBack) {
        hubState.otaRollbackOccurred = true;
        otaPrefs.putBool("rollback", true);
        otaPrefs.remove("trial_active");
        otaPrefs.putString("failed_part", failedPart);
        Serial.printf("[OTA Fail-Safe] ⚠️ Automatic rollback detected! System safely recovered to %s.\n",
                      hubState.activePartition.c_str());
    } else {
        hubState.otaRollbackOccurred = otaPrefs.getBool("rollback", false);
        if (hubState.otaRollbackOccurred) {
            Serial.println("[OTA Fail-Safe] System running on recovered partition after automatic rollback.");
        }
    }
    otaPrefs.end();

    // Tier-1 Auto-Rollback Crash Watchdog (active ONLY during unverified OTA trial)
    bool isOtaTrial = (hubState.otaPendingVerify || trialActive);

    if (isOtaTrial) {
        // Cold power-on reset (ESP_RST_POWERON) is a manual user power switch, not an in-flight crash
        if (rstReason != ESP_RST_POWERON) {
            rtcBootFailures++;
            Serial.printf("[OTA Fail-Safe] Active Trial Crash Counter: %d / 3 (Reason: %d)\n", rtcBootFailures, (int)rstReason);
        } else {
            Serial.println("[OTA Fail-Safe] Cold power-on detected during trial (preserving stability window).");
        }

        if (rtcBootFailures >= 3) {
            Serial.println("\n[OTA Fail-Safe] ⚠️ 3 CONSECUTIVE CRASHES DETECTED DURING OTA TRIAL!");
            Serial.println("[OTA Fail-Safe] Initiating emergency hardware rollback to previous stable partition...");

            otaPrefs.begin("aura_ota", false);
            otaPrefs.putBool("rollback", true);
            otaPrefs.putString("failed_part", hubState.activePartition);
            otaPrefs.remove("trial_active");
            otaPrefs.end();

            rtcBootFailures = 0; // Reset counter for the fallback partition

            if (esp_ota_check_rollback_is_possible()) {
                esp_ota_mark_app_invalid_rollback_and_reboot();
            } else {
                Serial.println("[OTA Fail-Safe] Rollback partition not bootable; staying on current partition.");
            }
        }
    } else {
        // System is running on a permanently verified stable partition.
        // Routine reboots, brownouts, or power switches NEVER trigger rollback!
        rtcBootFailures = 0;
    }

    // 1. Initialize Mutex
    stateMutex = xSemaphoreCreateMutex();

    // 2. Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed. Formatting...");
    } else {
        Serial.println("[FS] LittleFS Mounted Successfully.");
    }

    // 3. Load Network Credentials safely from NVS
    Preferences bootPrefs;
    bootPrefs.begin("aura", false);
    activeSsid = bootPrefs.getString("wifiSsid", "");
    activePass = bootPrefs.getString("wifiPass", "");
    bootPrefs.end();

    if (activeSsid.length() == 0) {
        activeSsid = DEFAULT_WIFI_SSID;
        activePass = DEFAULT_WIFI_PASSWORD;
    }

    // 4. Load Full Persistent State from NVS (Ensures Hub returns to exact previous state_t0)
    webManager.loadPersistentState();

    bool systemWasSwitchedOff = (!hubState.isMasterOn || !hubState.desk.power);
    Serial.printf("[Boot] Restored previous power state: %s (Master: %s | Desk: %s)\n",
                  systemWasSwitchedOff ? "OFF" : "ON",
                  hubState.isMasterOn ? "ON" : "OFF",
                  hubState.desk.power ? "ON" : "OFF");

    // 5. Initialize Hardware LED Strip
    ledCtrl.initHardware();

    // 6. ALWAYS play Apple Siri HomePod Physics Fluid Wave on boot!
    // Clamped to 450mA (<220mA total current draw). Provides immediate visual confirmation that ESP32 is powered and alive.
    ledCtrl.playSiriBootupAnimation(2500);

    // 7. Network Bring-Up:
    bool wifiConnected = false;
    if (activeSsid.length() > 0) {
        Serial.printf("[WiFi] Initiating connection to '%s'...\n", activeSsid.c_str());
        WiFi.setTxPower(WIFI_POWER_17dBm); // Prevent high-current RF inrush spikes
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFi.begin(activeSsid.c_str(), activePass.c_str());

        // Probe for Wi-Fi connection (max 30 attempts * 150ms = 4.5s)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(150);
            yield();
            Serial.print(".");
            attempts++;
        }
        wifiConnected = (WiFi.status() == WL_CONNECTED);
    }

    if (wifiConnected) {
        Serial.println("\n[WiFi] Connected successfully to home network!");
        Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());

        hubState.wifiSsid = activeSsid;
        hubState.wifiRssi = WiFi.RSSI();
        hubState.isSetupMode = false;

        WiFi.setSleep(false); // Low latency on local network

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

        // Restore full operational power budget
        FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
    } else {
        Serial.println("\n[WiFi] Station not connected yet (Router may be starting). Entering Autonomous AP+STA Mode...");
        hubState.isSetupMode = true;
        hubState.wifiSsid = activeSsid;
        hubState.wifiRssi = 0;

        // Start AP+STA mode to broadcast setup hotspot while background station reconnection continues
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS);
        Serial.printf("[WiFi] SoftAP started! SSID: '%s' | IP: %s\n", SOFTAP_SSID, WiFi.softAPIP().toString().c_str());

        // Start Captive Portal DNS on port 53 (redirects all domains to 192.168.4.1)
        webManager.enableCaptivePortal();

        // Amber breathing beacon is only activated if no network was configured (fresh setup)
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

    // 8. Initialize WiZ Controller (UDP) with HubState binding
    wizCtrl.begin(&hubState);

    // 9. Initialize Web Server, Captive Portal, and WebSockets (Port 80 ready)
    webManager.begin();

    // Trigger background UDP discovery on boot if connected to local LAN
    if (wifiConnected) {
        Serial.println("[WiZ] Triggering boot-time UDP discovery broadcast & initial state sync...");
        wizCtrl.broadcastDiscovery();
        wizCtrl.syncAllBulbs();
    }

    // 10. Spawn Core 1 FreeRTOS Task for LED Rendering at the very end of setup
    xTaskCreatePinnedToCore(
        LedTask,
        "LedTask",
        8192,
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

            // Switch cleanly to pure station mode (softAPdisconnect false does NOT stop WiFi)
            WiFi.softAPdisconnect(false);
            WiFi.enableAP(false);
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

    // Tier-1 Autonomous Dual-OTA Stability Trial Guard (20-Second Verification Window)
    static bool otaTrialPassed = false;
    static uint32_t bootTimestamp = millis();
    if (!otaTrialPassed && (millis() - bootTimestamp >= 20000)) {
        otaTrialPassed = true;
        rtcBootFailures = 0; // Successfully ran stably for 20s! Reset crash counter!

        Preferences otaPrefs;
        otaPrefs.begin("aura_ota", false);
        otaPrefs.remove("trial_active");
        otaPrefs.end();

        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_ota_img_states_t otaState;
        if (running != NULL && esp_ota_get_state_partition(running, &otaState) == ESP_OK) {
            if (otaState == ESP_OTA_IMG_PENDING_VERIFY) {
                esp_ota_mark_app_valid_cancel_rollback();
                Serial.println("[OTA Fail-Safe] 20-second stability trial passed. Partition marked VALID.");
            }
        }
        if (hubState.otaPendingVerify) {
            hubState.otaPendingVerify = false;
            webManager.broadcastState();
        }
    }

    delay(20);
}
