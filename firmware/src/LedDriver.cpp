#include "LedDriver.h"

LedDriver::LedDriver() {}

void LedDriver::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setDither(0); // DISABLE TEMPORAL DITHERING: Completely kills 50Hz/60Hz AC-like glow shimmer!
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000); // 1.0A smart safety limiter - stops tail-end (LEDs 50-60) voltage sag on high-current effects!
    FastLED.setBrightness(180);

    Serial.println("\n=======================================================");
    Serial.println("   [LED HARDWARE TEST] RUNNING 4-STAGE STARTUP CHECK   ");
    Serial.printf("   Active Pin: GPIO %d (D4) | %d LEDs | Dither: OFF\n", LED_PIN, NUM_LEDS);
    Serial.println("=======================================================");

    // Stage 1: All 60 LEDs RED
    Serial.println("[LED Test 1/4] >>> ALL 60 LEDS -> SOLID RED <<<");
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(800);

    // Stage 2: All 60 LEDs GREEN
    Serial.println("[LED Test 2/4] >>> ALL 60 LEDS -> SOLID GREEN <<<");
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(800);

    // Stage 3: All 60 LEDs BLUE
    Serial.println("[LED Test 3/4] >>> ALL 60 LEDS -> SOLID BLUE <<<");
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
    delay(800);

    // Stage 4: All 60 LEDs WHITE
    Serial.println("[LED Test 4/4] >>> ALL 60 LEDS -> SOLID WHITE <<<");
    fill_solid(leds, NUM_LEDS, CRGB(180, 180, 180));
    FastLED.show();
    delay(800);

    FastLED.clear(true);
    Serial.println("[LED Test] Startup Hardware Check Complete! Entering Live Rendering.\n");
}

void LedDriver::effectTestCycle() {
    FastLED.setBrightness(255);
    uint32_t sec = (millis() / 1000) % 5;
    if (sec == 0) {
        fill_solid(leds, NUM_LEDS, CRGB::Red);
    } else if (sec == 1) {
        fill_solid(leds, NUM_LEDS, CRGB::Green);
    } else if (sec == 2) {
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
    } else if (sec == 3) {
        fill_solid(leds, NUM_LEDS, CRGB::White);
    } else {
        // High-visibility chase dot back and forth
        fill_solid(leds, NUM_LEDS, CRGB(20, 20, 20));
        int pos = beatsin16(40, 0, NUM_LEDS - 1);
        for (int o = -2; o <= 2; o++) {
            int idx = pos + o;
            if (idx >= 0 && idx < NUM_LEDS) leds[idx] = CRGB::Cyan;
        }
    }
}

void LedDriver::update(const DeviceDesk& config, bool masterOn, uint8_t masterBrightness, int8_t contrastRatio) {
    uint32_t now = millis();
    // 60 FPS update rate guard (approx 16ms interval, relaxed to 10ms for jitter-free scheduler)
    if (now - lastUpdate < 10) return;
    lastUpdate = now;
    animationStep++;

    // Calculate effective brightness based on Master power, Master brightness, and Contrast ratio
    if (!masterOn || !config.power) {
        if (lastMasterOn || lastDeskPower) {
            FastLED.clear(true);
            lastMasterOn = false;
            lastDeskPower = false;
        }
        return;
    }

    float masterFactor = (float)masterBrightness / 100.0f;
    float contrastFactor = 1.0f + ((float)contrastRatio / 100.0f);
    if (contrastFactor < 0.0f) contrastFactor = 0.0f;
    if (contrastFactor > 1.5f) contrastFactor = 1.5f;

    float effective = ((float)config.brightness / 100.0f) * masterFactor * contrastFactor;
    uint8_t targetDim = constrain((int)(effective * 255.0f), 0, 255);

    CRGB baseColor = CRGB(config.r, config.g, config.b);

    // 1. Single LED Inspection Mode (Diagnose individual LEDs 0-59 one-by-one)
    if (config.inspectIndex >= 0 && config.inspectIndex < NUM_LEDS) {
        if (config.inspectIndex != lastInspectIndex || baseColor != lastSolidColor || targetDim != lastDeskDim) {
            lastInspectIndex = config.inspectIndex;
            lastSolidColor = baseColor;
            lastDeskDim = targetDim;
            FastLED.clear();
            leds[config.inspectIndex] = baseColor;
            FastLED.setBrightness(targetDim);
            FastLED.show();
            Serial.printf("[LED Inspect] Solitary Pixel Active: LED #%d | RGB(%d,%d,%d)\n", 
                          config.inspectIndex, config.r, config.g, config.b);
        }
        return;
    }
    lastInspectIndex = -1;

    // 2. Static Solid Color Mode: Update ONLY on change or 3s keepalive (Stops 60Hz bus hammering & flicker!)
    if (config.effect == "solid") {
        bool changed = (baseColor != lastSolidColor || targetDim != lastDeskDim || 
                        !lastMasterOn || !lastDeskPower || lastEffect != "solid" || 
                        now - lastSolidPush > 3000);
        if (changed) {
            lastSolidColor = baseColor;
            lastDeskDim = targetDim;
            lastMasterOn = masterOn;
            lastDeskPower = config.power;
            lastEffect = "solid";
            lastSolidPush = now;
            effectSolid(baseColor, targetDim);
            FastLED.show();
        }
        return;
    }
    lastEffect = config.effect;

    // Periodic heartbeat to Serial to confirm Core 1 rendering
    static uint32_t lastHeartbeat = 0;
    if (now - lastHeartbeat > 3000) {
        lastHeartbeat = now;
        Serial.printf("[LED Task] Dynamic Rendering Active | Effect: %s | Dim: %d/255 | Step: %u\n", 
                      config.effect.c_str(), targetDim, animationStep);
    }

    if (config.effect == "test-cycle") {
        effectTestCycle();
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
    } else if (config.effect == "caustics") {
        effectCaustics(baseColor, targetDim, config.speed, config.intensity);
    } else if (config.effect == "soliton") {
        effectSoliton(baseColor, targetDim, config.speed, config.intensity);
    } else if (config.effect == "thermal-plume") {
        effectThermalPlume(baseColor, targetDim, config.speed, config.intensity);
    } else if (config.effect == "marangoni") {
        effectMarangoni(baseColor, targetDim, config.speed, config.intensity);
    } else {
        effectSolid(baseColor, targetDim);
    }

    // Dampen jitter on pixel 36 to stabilize physical silicon micro-glitch
    if (NUM_LEDS > 36) {
        leds[36] = blend(lastLed36, leds[36], 190);
        lastLed36 = leds[36];
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
    CRGB magenta = CRGB(210, 50, 130);
    CRGB cyan = CRGB(0, 190, 210);

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

void LedDriver::effectCaustics(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity) {
    FastLED.setBrightness(brightness);
    uint8_t step = animationStep * (speed / 12 + 1);
    uint8_t turb = map(intensity, 0, 100, 20, 80);

    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t w1 = sin8((i * 14) - (step * 2));
        uint8_t w2 = sin8((i * 22) + (step * 3));
        uint8_t caustic = (w1 + w2) / 2;
        // High contrast ray bundling threshold for caustic highlights
        if (caustic > 175) {
            uint8_t boost = map(caustic, 175, 255, 0, turb * 2);
            leds[i] = blend(baseColor, CRGB::White, boost);
        } else {
            uint8_t dim = map(caustic, 0, 175, 140, 255);
            leds[i] = baseColor;
            leds[i].nscale8_video(dim);
        }
    }
}

void LedDriver::effectSoliton(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity) {
    FastLED.setBrightness(brightness);
    fill_solid(leds, NUM_LEDS, baseColor);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].nscale8_video(70); // Deep coherent base
    }

    uint8_t bpm = map(speed, 0, 100, 15, 65);
    int pos1 = beatsin16(bpm, 0, NUM_LEDS - 1);
    int pos2 = beatsin16(bpm * 4 / 3, 0, NUM_LEDS - 1);

    // Soliton 1 localized packet
    for (int offset = -4; offset <= 4; offset++) {
        int idx = pos1 + offset;
        if (idx >= 0 && idx < NUM_LEDS) {
            uint8_t mag = 255 - (abs(offset) * 55);
            leds[idx] += CHSV(180, 220, mag);
        }
    }

    // Soliton 2 localized packet
    for (int offset = -4; offset <= 4; offset++) {
        int idx = pos2 + offset;
        if (idx >= 0 && idx < NUM_LEDS) {
            uint8_t mag = 255 - (abs(offset) * 55);
            leds[idx] += CHSV(280, 200, mag);
        }
    }
}

void LedDriver::effectThermalPlume(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity) {
    FastLED.setBrightness(brightness);
    uint16_t offset = animationStep * (speed / 8 + 1);
    uint8_t scale = map(intensity, 0, 100, 15, 60);

    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t noise1 = inoise8(i * scale, offset);
        uint8_t noise2 = inoise8(i * (scale * 2), offset * 2);
        uint8_t fbm = (noise1 * 2 + noise2) / 3;

        CRGB plume = HeatColor(fbm);
        leds[i] = blend(baseColor, plume, 160);
    }
}

void LedDriver::effectMarangoni(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity) {
    FastLED.setBrightness(brightness);
    uint8_t phase = animationStep * (speed / 14 + 1);

    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t cell = sin8((i * 256 * 4) / NUM_LEDS);
        uint8_t swirl = cos8(phase + (cell / 2));
        uint8_t blendFactor = (cell + swirl) / 2;

        CRGB secondary = baseColor;
        secondary.r = qadd8(secondary.r, 45);
        secondary.b = qadd8(secondary.b, 65);

        leds[i] = blend(baseColor, secondary, blendFactor);
    }
}
