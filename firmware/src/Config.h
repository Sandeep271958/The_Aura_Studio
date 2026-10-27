#pragma once

#include <Arduino.h>

// ============================================================================
// WI-FI & NETWORK CONFIGURATION
// ============================================================================
#define DEFAULT_WIFI_SSID "YOUR_WIFI_SSID"
#define DEFAULT_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define HOSTNAME "aura-hub" // Accessible at http://aura-hub.local

// ============================================================================
// HARDWARE: WS2812B DESK STRIP
// ============================================================================
#define LED_PIN 16   // GPIO 16 (Safe pin, non-strapping)
#define NUM_LEDS 120 // Adjust to match your strip length
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define MAX_POWER_MILLIAMPS 3000 // Safety limit: 3000mA (3 Amps)
#define TARGET_FPS 60            // Smooth 60 FPS animation loop

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
  bool isMasterOn = true;
  uint8_t masterBrightness = 80;
  int8_t contrastRatio = 0; // -50 to +50
  bool syncCorners = true;
  String activePreset = "cyberpunk";

  String wizLeftIp = DEFAULT_WIZ_LEFT_IP;
  String wizRightIp = DEFAULT_WIZ_RIGHT_IP;

  DeviceDesk desk;
  DeviceWiz wizLeft;
  DeviceWiz wizRight;
};
