#pragma once

#include "Config.h"
#include <Arduino.h>
#include <FastLED.h>

class LedDriver {
public:
  LedDriver();
  void begin();
  void update(const DeviceDesk &config, bool masterOn, uint8_t masterBrightness,
              int8_t contrastRatio);

private:
  CRGB leds[NUM_LEDS];
  uint32_t lastUpdate = 0;
  uint16_t animationStep = 0;

  // Anti-flicker state tracking for static/solid rendering
  bool lastMasterOn = false;
  bool lastDeskPower = false;
  uint8_t lastDeskDim = 0;
  CRGB lastSolidColor = CRGB::Black;
  String lastEffect = "";
  int lastInspectIndex = -2;
  uint32_t lastSolidPush = 0;
  CRGB lastLed36 = CRGB::Black;

  // Effect implementations
  void effectTestCycle();
  void effectSolid(CRGB targetColor, uint8_t brightness);
  void effectBreathe(CRGB baseColor, uint8_t brightness, uint8_t speed);
  void effectCyberpulse(uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectSunsetWave(uint8_t brightness, uint8_t speed);
  void effectAurora(uint8_t brightness, uint8_t speed);
  void effectFire(uint8_t brightness, uint8_t speed);
  void effectSoapBubble(uint8_t brightness, uint8_t speed);
  void effectWave(CRGB baseColor, uint8_t brightness, uint8_t speed);
  void effectShimmer(CRGB baseColor, uint8_t brightness, uint8_t speed);
  void effectCaustics(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectSoliton(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectThermalPlume(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectMarangoni(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
};
