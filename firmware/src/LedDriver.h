#pragma once

#include "Config.h"
#include <Arduino.h>
#include <FastLED.h>

class LedDriver {
public:
  LedDriver();
  void begin();
  void initHardware();
  void playSiriBootupAnimation(uint32_t durationMs = 2500);
  void update(const DeviceDesk &config, bool masterOn, uint8_t masterBrightness,
              int8_t contrastRatio, bool useGammaCorrection = true,
              const String &onOffAnimation = "gaussian-ripple");

private:
  CRGB leds[NUM_LEDS];
  CRGB snapshotLeds[NUM_LEDS];
  uint32_t lastUpdate = 0;
  uint16_t animationStep = 0;

  // Power transition animation state machine
  enum PowerTransitionState { PWR_OFF, PWR_TURNING_ON, PWR_ON, PWR_TURNING_OFF };
  PowerTransitionState pwrState = PWR_OFF;
  uint32_t transitionStartTime = 0;
  uint16_t transitionDuration = 1000;
  String currentAnimMode = "gaussian-ripple";

  void renderOnAnimation(float t, const String &mode);
  void renderOffAnimation(float t, const String &mode, bool useGammaCorrection);

  // Anti-flicker state tracking for static/solid rendering
  bool lastMasterOn = false;
  bool lastDeskPower = false;
  uint8_t lastDeskDim = 0;
  CRGB lastSolidColor = CRGB::Black;
  char lastEffect[32] = "";
  int lastInspectIndex = -2;
  uint32_t lastSolidPush = 0;
  CRGB lastLed36 = CRGB::Black;
  bool lastUseGamma = true;

  // Helper functions
  static CRGB interpolate4Stops(const CRGB stops[4], float t);
  void effectGradientPhysics(const DeviceDesk &config);

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
  void effectSpatialFlow(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectCelestialDrift(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectEmberHearth(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectChromaticPrism(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectBioluminescentTide(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
  void effectNordicAurora(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity);
};
