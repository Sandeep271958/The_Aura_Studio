#include "LedDriver.h"

LedDriver::LedDriver() {}

void LedDriver::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.clear(true);
    Serial.printf("[LED] FastLED initialized on GPIO %d with %d LEDs\n", LED_PIN, NUM_LEDS);
}

void LedDriver::update(const DeviceDesk& config, bool masterOn, uint8_t masterBrightness, int8_t contrastRatio) {
    uint32_t now = millis();
    // 60 FPS update rate (approx 16ms interval)
    if (now - lastUpdate < 16) return;
    lastUpdate = now;
    animationStep++;

    // Calculate effective brightness based on Master power, Master brightness, and Contrast ratio
    if (!masterOn || !config.power) {
        FastLED.clear(true);
        return;
    }

    float masterFactor = (float)masterBrightness / 100.0f;
    float contrastFactor = 1.0f + ((float)contrastRatio / 100.0f);
    if (contrastFactor < 0.0f) contrastFactor = 0.0f;
    if (contrastFactor > 1.5f) contrastFactor = 1.5f;

    float effective = ((float)config.brightness / 100.0f) * masterFactor * contrastFactor;
    uint8_t targetDim = constrain((int)(effective * 255.0f), 0, 255);

    CRGB baseColor = CRGB(config.r, config.g, config.b);

    if (config.effect == "solid") {
        effectSolid(baseColor, targetDim);
    } else if (config.effect == "breathe") {
        effectBreathe(baseColor, targetDim, config.speed);
    } else if (config.effect == "cyberpulse") {
        effectCyberpulse(targetDim, config.speed, config.intensity);
    } else if (config.effect == "sunset-wave") {
        effectSunsetWave(targetDim, config.speed);
    } else if (config.effect == "aurora") {
        effectAurora(targetDim, config.speed);
    } else if (config.effect == "fire") {
        effectFire(targetDim, config.speed);
    } else if (config.effect == "soap-bubble") {
        effectSoapBubble(targetDim, config.speed);
    } else if (config.effect == "wave") {
        effectWave(baseColor, targetDim, config.speed);
    } else if (config.effect == "shimmer") {
        effectShimmer(baseColor, targetDim, config.speed);
    } else {
        effectSolid(baseColor, targetDim);
    }

    FastLED.show();
}

void LedDriver::effectSolid(CRGB targetColor, uint8_t brightness) {
    FastLED.setBrightness(brightness);
    fill_solid(leds, NUM_LEDS, targetColor);
}

void LedDriver::effectBreathe(CRGB baseColor, uint8_t brightness, uint8_t speed) {
    uint8_t breathWave = sin8(animationStep * (speed / 15 + 1));
    uint8_t scaledDim = scale8(brightness, map(breathWave, 0, 255, 60, 255));
    FastLED.setBrightness(scaledDim);
    fill_solid(leds, NUM_LEDS, baseColor);
}

void LedDriver::effectCyberpulse(uint8_t brightness, uint8_t speed, uint8_t intensity) {
    FastLED.setBrightness(brightness);
    CRGB magenta = CRGB(236, 72, 153);
    CRGB cyan = CRGB(0, 242, 254);

    uint8_t waveOffset = animationStep * (speed / 12 + 1);
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t blendVal = sin8((i * (intensity / 10 + 1)) + waveOffset);
        leds[i] = blend(magenta, cyan, blendVal);
    }
}

void LedDriver::effectSunsetWave(uint8_t brightness, uint8_t speed) {
    FastLED.setBrightness(brightness);
    CRGB amber = CRGB(245, 158, 11);
    CRGB peach = CRGB(244, 63, 94);
    CRGB warmGold = CRGB(255, 180, 50);

    uint8_t waveOffset = animationStep * (speed / 15 + 1);
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t blend1 = sin8((i * 4) + waveOffset);
        CRGB mixed = blend(amber, peach, blend1);
        leds[i] = blend(mixed, warmGold, sin8(i * 2 + waveOffset));
    }
}

void LedDriver::effectAurora(uint8_t brightness, uint8_t speed) {
    FastLED.setBrightness(brightness);
    CRGB emerald = CRGB(16, 185, 129);
    CRGB violet = CRGB(139, 92, 246);
    CRGB deepTeal = CRGB(4, 120, 87);

    uint8_t waveOffset = animationStep * (speed / 18 + 1);
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t bVal = sin8((i * 6) + waveOffset);
        CRGB grad = blend(emerald, violet, bVal);
        leds[i] = blend(grad, deepTeal, cos8(i * 3 + waveOffset));
    }
}

void LedDriver::effectFire(uint8_t brightness, uint8_t speed) {
    FastLED.setBrightness(brightness);
    static uint8_t heat[NUM_LEDS];

    // Cool down every cell a little
    for (int i = 0; i < NUM_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((speed * 10) / NUM_LEDS) + 2));
    }

    // Heat from each cell drifts up and diffuses
    for (int k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Randomly ignite new sparks near bottom
    if (random8() < 160) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Map heat to LED colors
    for (int j = 0; j < NUM_LEDS; j++) {
        CRGB color = HeatColor(heat[j]);
        leds[j] = color;
    }
}

void LedDriver::effectSoapBubble(uint8_t brightness, uint8_t speed) {
    FastLED.setBrightness(brightness);
    uint8_t phaseShift = animationStep * (speed / 16 + 1);

    for (int i = 0; i < NUM_LEDS; i++) {
        // Multi-harmonic thin film interference simulation
        uint8_t wave1 = sin8((i * 5) - phaseShift);
        uint8_t wave2 = cos8((i * 3) + (phaseShift / 2));
        uint8_t hue = (wave1 + wave2) / 2;
        // Soft pastel saturation (130-170) for iridescent soap bubble pearl look
        uint8_t sat = 135 + (sin8(i * 2 + phaseShift) / 8);
        leds[i] = CHSV(hue, sat, 255);
    }
}

void LedDriver::effectWave(CRGB baseColor, uint8_t brightness, uint8_t speed) {
    FastLED.setBrightness(brightness);
    uint8_t waveOffset = animationStep * (speed / 14 + 1);

    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t wave = sin8((i * 4) + waveOffset);
        leds[i] = blend(baseColor, CRGB::White, wave / 3);
    }
}

void LedDriver::effectShimmer(CRGB baseColor, uint8_t brightness, uint8_t speed) {
    FastLED.setBrightness(brightness);
    fill_solid(leds, NUM_LEDS, baseColor);

    uint8_t sparkChance = map(speed, 0, 100, 15, 60);
    for (int i = 0; i < NUM_LEDS; i++) {
        if (random8() < sparkChance) {
            leds[i] += CRGB(70, 70, 70);
        }
    }
}
