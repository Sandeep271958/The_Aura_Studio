#pragma once

#include <Arduino.h>
#include <FastLED.h>

// Global FreeRTOS Mutex for thread-safe state access across Core 0 (WiFi/Web)
// and Core 1 (FastLED)
extern SemaphoreHandle_t stateMutex;

// ============================================================================
// FIRMWARE NOMENCLATURE & REVISION (Industry Standard XX.YY.ZZZ)
// ============================================================================
#define FIRMWARE_VERSION "02.00.000"
#define FIRMWARE_NAME "firmware_02.00.000.bin"
#define FIRMWARE_BUILD "2026-11-19"

// ============================================================================
// WI-FI & NETWORK CONFIGURATION
// ============================================================================
#define DEFAULT_WIFI_SSID "1201_2.4G"
#define DEFAULT_WIFI_PASSWORD "V@08101999g"
#define SOFTAP_SSID "Aura-Studio-Setup"
#define SOFTAP_PASS                                                            \
  "" // Open setup hotspot for seamless captive portal onboarding
#define HOSTNAME "aura-hub" // Accessible at http://aura-hub.local

// ============================================================================
// HARDWARE: WS2812B DESK STRIP & SPATIAL ORIENTATION
// ============================================================================
#define FASTLED_SHOW_CORE                                                      \
  1                 // Bind FastLED show to Core 1 away from Wi-Fi interrupts
#define LED_PIN 4   // Active: GPIO 4 (Labeled D4 on NodeMCU ESP32)
#define NUM_LEDS 60 // 1 meter strip with 60 LEDs (Indices 0 to 59)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define MAX_POWER_MILLIAMPS 1500 // Safety power limit: 1.5A for 60 LEDs
#define TARGET_FPS 60            // Smooth 60 FPS animation loop

// Physical Hardware Spatial Orientation:
// - LED 1  [Index 0]  is located near the Left LED Lamp (wizLeft)
// - LED 60 [Index 59] is located near the Right LED Lamp (wizRight)
#define STRIP_LEFT_INDEX 0   // Left Corner Lamp Edge (Index 0)
#define STRIP_RIGHT_INDEX 59 // Right Corner Lamp Edge (Index 59)

// ============================================================================
// PHILIPS WIZ LOCAL PROTOCOL
// ============================================================================
#define WIZ_UDP_PORT 38899
#define DEFAULT_WIZ_LEFT_IP "192.168.1.101"
#define DEFAULT_WIZ_RIGHT_IP "192.168.1.102"

// ============================================================================
// STATE DATA STRUCTURES
// ============================================================================
struct DeviceDesk {
  bool power = true;
  uint8_t brightness = 85;
  uint8_t r = 236;
  uint8_t g = 72;
  uint8_t b = 153;
  String effect = "cyberpulse";
  uint8_t speed = 60;
  uint8_t intensity = 75;
  int inspectIndex =
      -1; // -1 for normal animation, 0 to NUM_LEDS-1 for single-LED diagnostic

  // 4-Stop Luxury Gradient Lab Palette Support
  bool hasGradient = false;
  CRGB stops[4] = {
      CRGB(165, 243, 252), // Stop 1 (#a5f3fc)
      CRGB(244, 114, 182), // Stop 2 (#f472b6)
      CRGB(192, 132, 252), // Stop 3 (#c084fc)
      CRGB(254, 240, 138)  // Stop 4 (#fef08a)
  };
};

struct DeviceWiz {
  bool power = true;
  uint8_t brightness = 75;
  String mode = "rgb"; // "rgb" or "cct"
  uint8_t r = 30;
  uint8_t g = 64;
  uint8_t b = 175;
  uint16_t kelvin = 2700;
};

struct HubState {
  String firmwareVersion = FIRMWARE_VERSION;
  bool isMasterOn = true;
  uint8_t masterBrightness = 80;
  int8_t contrastRatio = 0; // -50 to +50
  bool syncCorners = true;
  String activePreset = "cyberpunk";
  bool useGammaCorrection = true;
  String onOffAnimation = "gaussian-ripple";

  String wizLeftIp = DEFAULT_WIZ_LEFT_IP;
  String wizRightIp = DEFAULT_WIZ_RIGHT_IP;
  String wizLeftMac = "";
  String wizRightMac = "";

  // Dynamic Wi-Fi & SoftAP Provisioning State
  String wifiSsid = "";
  int8_t wifiRssi = 0;
  bool isSetupMode = false;

  // Dual-OTA Partition & Rollback Telemetry
  String activePartition = "app0"; // "app0" (ota_0) or "app1" (ota_1)
  bool otaRollbackOccurred = false;
  bool otaPendingVerify = false;

  // Zero-Allocation Rendering Generation Counter
  uint32_t deskVersion = 0;

  DeviceDesk desk;
  DeviceWiz wizLeft;
  DeviceWiz wizRight;
};
