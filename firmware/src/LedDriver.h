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

  // Effect implementations
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
