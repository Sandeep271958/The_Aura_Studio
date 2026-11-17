#include "LedDriver.h"
#include <cmath>
#include <esp_task_wdt.h>

// 256-Entry CIE 1931 Perceptual Lightness Lookup Table: Y = ((L* + 16)/116)^3
const uint8_t PROGMEM cie1931[256] = {
      0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   3,   4,
      4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   7,
      7,   7,   7,   8,   8,   8,   8,   9,   9,   9,  10,  10,  10,  10,  11,  11,
     11,  12,  12,  12,  13,  13,  13,  14,  14,  15,  15,  15,  16,  16,  17,  17,
     17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  23,  24,  24,  25,
     25,  26,  26,  27,  28,  28,  29,  29,  30,  31,  31,  32,  32,  33,  34,  34,
     35,  36,  37,  37,  38,  39,  39,  40,  41,  42,  43,  43,  44,  45,  46,  47,
     47,  48,  49,  50,  51,  52,  53,  54,  54,  55,  56,  57,  58,  59,  60,  61,
     62,  63,  64,  65,  66,  67,  68,  70,  71,  72,  73,  74,  75,  76,  77,  79,
     80,  81,  82,  83,  85,  86,  87,  88,  90,  91,  92,  94,  95,  96,  98,  99,
    100, 102, 103, 105, 106, 108, 109, 110, 112, 113, 115, 116, 118, 120, 121, 123,
    124, 126, 128, 129, 131, 132, 134, 136, 138, 139, 141, 143, 145, 146, 148, 150,
    152, 154, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 175, 177, 179, 181,
    183, 185, 187, 189, 191, 193, 196, 198, 200, 202, 204, 207, 209, 211, 214, 216,
    218, 220, 223, 225, 228, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252, 255
};

CRGB LedDriver::interpolate4Stops(const CRGB stops[4], float t) {
    if (isnan(t)) return stops[0];
    if (t <= 0.0f) return stops[0];
    if (t >= 1.0f) return stops[3];
    float f;
    int idx0, idx1;
    if (t <= 0.333333f) {
        idx0 = 0; idx1 = 1;
        f = t / 0.333333f;
    } else if (t <= 0.666667f) {
        idx0 = 1; idx1 = 2;
        f = (t - 0.333333f) / 0.333334f;
    } else {
        idx0 = 2; idx1 = 3;
        f = (t - 0.666667f) / 0.333333f;
    }
    uint8_t r = (uint8_t)(stops[idx0].r + (stops[idx1].r - stops[idx0].r) * f + 0.5f);
    uint8_t g = (uint8_t)(stops[idx0].g + (stops[idx1].g - stops[idx0].g) * f + 0.5f);
    uint8_t b = (uint8_t)(stops[idx0].b + (stops[idx1].b - stops[idx0].b) * f + 0.5f);
    return CRGB(r, g, b);
}

LedDriver::LedDriver() {}

void LedDriver::begin() {
    initHardware();
    playSiriBootupAnimation(2500);
}

void LedDriver::initHardware() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setTemperature(DirectSunlight); // Calibrate WS2812B raw blue phosphor to D65 daylight
    FastLED.setDither(0); // DISABLE TEMPORAL DITHERING: Completely kills 50Hz/60Hz AC-like glow shimmer!
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS); // 1500mA smart safety limiter
    FastLED.setBrightness(255); // Global scaler kept at 255; dimming handled in software via CIE 1931
    FastLED.clear(true);
}

void LedDriver::playSiriBootupAnimation(uint32_t durationMs) {
    Serial.println("\n=======================================================");
    Serial.println("   [AURA BOOTUP] APPLE SIRI HOMEPOD FLUID WAVE         ");
    Serial.printf("   Active Pin: GPIO %d | %d LEDs | Dither: OFF | %u ms\n", LED_PIN, NUM_LEDS, (unsigned int)durationMs);
    Serial.println("   Palette: Apple Intelligence (Orchid, Violet, Cyan, Coral, White)");
    Serial.println("   Physics: Conic Solitons, Frosted Glass Diffusion, Cosine Dissolve");
    Serial.println("=======================================================\n");

    // Apple Intelligence & HomePod "Sound Glow" Palette Stops
    const struct { float r, g, b; } siriStops[4] = {
        {236.0f,  72.0f, 153.0f},  // Stop 0: Electric Orchid / Neon Pink (#EC4899)
        {124.0f,  58.0f, 237.0f},  // Stop 1: Royal Violet / Deep Indigo  (#7C3AED)
        {  0.0f, 242.0f, 254.0f},  // Stop 2: Siri Electric Cyan / Azure  (#00F2FE)
        {251.0f, 146.0f,  60.0f}   // Stop 3: Sunset Amber / Warm Coral   (#FB923C)
    };

    struct RawColor { float r, g, b; };
    RawColor rawFrame[NUM_LEDS];

    FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); // Safe inrush current limiter (450mA peak) prevents USB brownouts
    uint32_t bootStart = millis();

    while (millis() - bootStart < durationMs) {
        esp_task_wdt_reset(); // Feed Task Watchdog Timer to prevent reboot loops during long boot sequence
        yield();
        uint32_t elapsed = millis() - bootStart;
        float tau = (float)elapsed / (float)durationMs; // 0.0 -> 1.0

        // 1. Wake & Settle Transition Curves (Apple HomePod Lifecycle)
        float wakeEnv = 1.0f;
        float wakeSpread = 1.0f;
        if (tau < 0.18f) {
            float f = tau / 0.18f;
            wakeEnv = f * f * (3.0f - 2.0f * f); // Hermite cubic awakening
            wakeSpread = 0.15f + 0.85f * wakeEnv;
        }

        float settleFactor = 0.0f;
        float lumaFade = 1.0f;
        if (tau > 0.82f) {
            float f = (tau - 0.82f) / 0.18f;
            settleFactor = f * f * (3.0f - 2.0f * f); // Smooth deceleration into equilibrium
            float cosFade = cosf(f * 1.5707963f);
            lumaFade = cosFade * cosFade; // Silky cosine-damped logarithmic fade
        }

        // Fluid time factor with velocity deceleration during settle phase
        float timeFactor = tau * (1.0f - 0.40f * settleFactor);

        // 2. Render Conic Multi-Wave Fluid Physics across all 60 LEDs
        for (int i = 0; i < NUM_LEDS; i++) {
            float x = (float)i / (float)(NUM_LEDS - 1); // 0.0 -> 1.0

            // Bilateral Hermite Wake Mask expanding outward from strip center (LEDs 29-30)
            float distCenter = fabsf(x - 0.5f) * 2.0f;
            float rawMask = (wakeSpread - distCenter * 0.70f) / 0.30f;
            if (rawMask < 0.0f) rawMask = 0.0f;
            if (rawMask > 1.0f) rawMask = 1.0f;
            float mask = rawMask * rawMask * (3.0f - 2.0f * rawMask) * wakeEnv * lumaFade;

            // Fluid Domain Warping (convection currents & vortex modulation)
            float warpAmp = 0.07f * (1.0f - 0.5f * settleFactor);
            float xWarp = x + warpAmp * sinf(6.2831853f * (1.5f * x - 2.0f * timeFactor)) +
                              (warpAmp * 0.6f) * cosf(6.2831853f * (2.2f * (1.0f - x) + 1.6f * timeFactor));

            // Rotating Apple Intelligence Harmonic Phase Field
            float phase = (xWarp * 1.15f - timeFactor * 1.4f + 0.18f * sinf(6.2831853f * (0.9f * xWarp - timeFactor)));
            phase = fmodf(phase, 1.0f);
            if (phase < 0.0f) phase += 1.0f;

            // Hermite-interpolated cyclic 4-stop Apple color palette sampling
            float scaledPhase = phase * 4.0f;
            int idx0 = (int)scaledPhase % 4;
            int idx1 = (idx0 + 1) % 4;
            float fStop = scaledPhase - (float)((int)scaledPhase);
            fStop = fStop * fStop * (3.0f - 2.0f * fStop); // Cubic smoothstep

            float baseR = siriStops[idx0].r + (siriStops[idx1].r - siriStops[idx0].r) * fStop;
            float baseG = siriStops[idx0].g + (siriStops[idx1].g - siriStops[idx0].g) * fStop;
            float baseB = siriStops[idx0].b + (siriStops[idx1].b - siriStops[idx0].b) * fStop;

            // Soliton 1: Electric Orchid wave (drifts left to right with boundary reflection)
            float p1 = 0.5f + 0.42f * sinf(6.2831853f * (1.15f * timeFactor - 0.25f));
            float s1 = expf(-0.5f * powf((xWarp - p1) / 0.18f, 2.0f));

            // Soliton 2: Siri Cyan wave (counter-drifts right to left)
            float p2 = 0.5f - 0.42f * cosf(6.2831853f * (1.35f * timeFactor));
            float s2 = expf(-0.5f * powf((xWarp - p2) / 0.19f, 2.0f));

            // Specular Diamond White Collision Core (constructive interference surge)
            float collision = s1 * s2;
            float rawSpec = (collision - 0.07f) / 0.24f;
            if (rawSpec < 0.0f) rawSpec = 0.0f;
            if (rawSpec > 1.0f) rawSpec = 1.0f;
            float specular = powf(rawSpec, 1.35f) * (1.0f - 0.70f * settleFactor);

            // Additive light amplitude synthesis with soft-start peak luminance guard (<220mA total draw)
            float amp = (0.70f + 0.35f * (s1 + s2)) * mask;
            float bootLumaScale = 0.40f;

            float r = (baseR * amp + specular * (255.0f - baseR) * mask) * bootLumaScale;
            float g = (baseG * amp + specular * (255.0f - baseG) * mask) * bootLumaScale;
            float b = (baseB * amp + specular * (255.0f - baseB) * mask) * bootLumaScale;

            // Soft saturation tone-mapping (protects chromaticity at peak luminance)
            float maxC = fmaxf(r, fmaxf(g, b));
            if (maxC > 255.0f) {
                float scale = 255.0f / maxC;
                r *= scale;
                g *= scale;
                b *= scale;
            }

            rawFrame[i] = {r, g, b};
        }

        // 3. Sub-Pixel Spatial Diffusion Filter (Simulates HomePod Frosted Glass Touch-Cap)
        // 3-tap low-pass blur kernel [0.15, 0.70, 0.15] seamlessly blurs discrete LED boundaries
        for (int i = 0; i < NUM_LEDS; i++) {
            RawColor cPrev = (i > 0) ? rawFrame[i - 1] : rawFrame[i];
            RawColor cCurr = rawFrame[i];
            RawColor cNext = (i < NUM_LEDS - 1) ? rawFrame[i + 1] : rawFrame[i];

            float diffR = cPrev.r * 0.15f + cCurr.r * 0.70f + cNext.r * 0.15f;
            float diffG = cPrev.g * 0.15f + cCurr.g * 0.70f + cNext.g * 0.15f;
            float diffB = cPrev.b * 0.15f + cCurr.b * 0.70f + cNext.b * 0.15f;

            // 4. CIE 1931 Human Perceptual Lightness Gamma Encoding
            uint8_t rByte = constrain((int)(diffR + 0.5f), 0, 255);
            uint8_t gByte = constrain((int)(diffG + 0.5f), 0, 255);
            uint8_t bByte = constrain((int)(diffB + 0.5f), 0, 255);

            leds[i] = CRGB(pgm_read_byte(&cie1931[rByte]),
                           pgm_read_byte(&cie1931[gByte]),
                           pgm_read_byte(&cie1931[bByte]));
        }

        // Silicon micro-glitch hardware protection for Pixel 36
        if (NUM_LEDS > 36) {
            leds[36] = blend(lastLed36, leds[36], 190);
            lastLed36 = leds[36];
        }

        FastLED.show();
        delay(16); // Fluid 60 FPS
    }

    FastLED.clear(true);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS); // Restore full operational power budget
    esp_task_wdt_reset();
    Serial.println("[Aura Bootup] Siri HomePod Fluid Sequence Complete! Resuming Live Functionality.\n");
}

void LedDriver::effectTestCycle() {
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

void LedDriver::renderOnAnimation(float t, const String &mode) {
    if (mode == "bilateral-bloom") {
        float s = t * t * (3.0f - 2.0f * t);
        float E = s * 32.0f;
        float W = 6.0f;
        for (int i = 0; i < NUM_LEDS; i++) {
            float dist = fabsf((float)i - 29.5f);
            float alpha = constrain((E - dist + W * 0.5f) / W, 0.0f, 1.0f);
            alpha = alpha * alpha * (3.0f - 2.0f * alpha);
            float coronaDist = dist - E;
            float corona = expf(-(coronaDist * coronaDist) / 8.0f) * (1.0f - t * 0.3f) * 0.5f;
            float factor = constrain(alpha + corona, 0.0f, 1.0f);
            if (t > 0.88f) {
                float settle = (t - 0.88f) / 0.12f;
                factor = factor * (1.0f - settle) + 1.0f * settle;
            }
            leds[i].r = (uint8_t)(leds[i].r * factor);
            leds[i].g = (uint8_t)(leds[i].g * factor);
            leds[i].b = (uint8_t)(leds[i].b * factor);
        }
    } else if (mode == "thermal-warmup") {
        float tempT = powf(t, 1.35f);
        CRGB filament(255, (uint8_t)(60 + tempT * 125), (uint8_t)(6 + tempT * 40));
        float brightFactor = powf(t, 1.25f);
        for (int i = 0; i < NUM_LEDS; i++) {
            float jitter = 1.0f + 0.1f * sinf(i * 1.6f);
            float localT = constrain(tempT * jitter, 0.0f, 1.0f);
            CRGB blended = blend(filament, leds[i], (uint8_t)(localT * 255));
            leds[i].r = (uint8_t)(blended.r * brightFactor);
            leds[i].g = (uint8_t)(blended.g * brightFactor);
            leds[i].b = (uint8_t)(blended.b * brightFactor);
        }
    } else if (mode == "quantum-sweep") {
        float headPos = t * 68.0f;
        for (int i = 0; i < NUM_LEDS; i++) {
            float dist = headPos - (float)i;
            float factor = 0.0f;
            if (dist >= 0.0f) {
                factor = 1.0f;
                if (dist < 5.0f) {
                    float pulse = (1.0f - dist / 5.0f) * 0.5f;
                    factor = constrain(1.0f + pulse, 0.0f, 1.5f);
                }
            } else if (dist > -2.5f) {
                factor = (dist + 2.5f) / 2.5f * 0.35f;
            }
            leds[i].r = constrain((int)(leds[i].r * factor), 0, 255);
            leds[i].g = constrain((int)(leds[i].g * factor), 0, 255);
            leds[i].b = constrain((int)(leds[i].b * factor), 0, 255);
        }
    } else {
        // Option 4: Gaussian Harmonic Ripple (DEFAULT)
        float R = 33.0f * sqrtf(t);
        float sigma = 4.5f + 16.0f * t;
        for (int i = 0; i < NUM_LEDS; i++) {
            float dist = fabsf((float)i - 29.5f);
            float u = (R - dist) / 2.8f;
            float stepFill = 1.0f / (1.0f + expf(-u));
            float waveDelta = dist - R;
            float crest = expf(-(waveDelta * waveDelta) / (2.0f * sigma * sigma)) * 
                          cosf(waveDelta * (6.28318f / 7.5f));
            crest *= (1.0f - t * 0.45f) * 0.55f;
            float modFactor = constrain(stepFill + crest, 0.0f, 1.0f);
            if (t > 0.85f) {
                float settle = (t - 0.85f) / 0.15f;
                modFactor = modFactor * (1.0f - settle) + 1.0f * settle;
            }
            leds[i].r = (uint8_t)(leds[i].r * modFactor);
            leds[i].g = (uint8_t)(leds[i].g * modFactor);
            leds[i].b = (uint8_t)(leds[i].b * modFactor);
        }
    }
}

void LedDriver::renderOffAnimation(float t, const String &mode, bool useGammaCorrection) {
    if (mode == "bilateral-bloom") {
        float s = t * t * (3.0f - 2.0f * t);
        float collapseRadius = (1.0f - s) * 31.0f;
        float W = 5.0f;
        float overallFade = 1.0f - t * 0.5f;
        for (int i = 0; i < NUM_LEDS; i++) {
            float dist = fabsf((float)i - 29.5f);
            float diff = collapseRadius - dist;
            float alpha = constrain((diff + W * 0.5f) / W, 0.0f, 1.0f);
            alpha = alpha * alpha * (3.0f - 2.0f * alpha);
            float factor = alpha * overallFade;
            leds[i].r = (uint8_t)(snapshotLeds[i].r * factor);
            leds[i].g = (uint8_t)(snapshotLeds[i].g * factor);
            leds[i].b = (uint8_t)(snapshotLeds[i].b * factor);
        }
    } else if (mode == "thermal-warmup") {
        float blueGreenDecay = powf(constrain(1.0f - t * 2.2f, 0.0f, 1.0f), 2.0f);
        float redDecay = powf(1.0f - t, 1.8f);
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = (uint8_t)(snapshotLeds[i].r * redDecay);
            leds[i].g = (uint8_t)(snapshotLeds[i].g * blueGreenDecay);
            leds[i].b = (uint8_t)(snapshotLeds[i].b * blueGreenDecay);
        }
    } else if (mode == "quantum-sweep") {
        float voidHead = (1.0f - t) * 65.0f;
        for (int i = 0; i < NUM_LEDS; i++) {
            float dist = (float)i - voidHead;
            float factor = 1.0f;
            if (dist >= 0.0f) {
                factor = 0.0f;
            } else if (dist > -4.5f) {
                float trans = (-dist) / 4.5f;
                factor = trans * trans;
            }
            leds[i].r = (uint8_t)(snapshotLeds[i].r * factor);
            leds[i].g = (uint8_t)(snapshotLeds[i].g * factor);
            leds[i].b = (uint8_t)(snapshotLeds[i].b * factor);
        }
    } else {
        // Option 4: Cosine-damped logarithmic fade (DEFAULT)
        float cosProgress = 0.5f * (1.0f + cosf(t * 3.14159265f));
        float decay = powf(cosProgress, 1.8f);
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = (uint8_t)(snapshotLeds[i].r * decay);
            leds[i].g = (uint8_t)(snapshotLeds[i].g * decay);
            leds[i].b = (uint8_t)(snapshotLeds[i].b * decay);
        }
    }
}

void LedDriver::update(const DeviceDesk& config, bool masterOn, uint8_t masterBrightness,
                       int8_t contrastRatio, bool useGammaCorrection, const String &onOffAnimation) {
    uint32_t now = millis();
    // 60 FPS update rate guard (approx 16ms interval, relaxed to 10ms for jitter-free scheduler)
    if (now - lastUpdate < 10) return;
    lastUpdate = now;
    animationStep++;

    bool targetPower = (masterOn && config.power);

    // 1. Single LED Inspection Mode (Diagnose individual LEDs 0-59 one-by-one)
    if (config.inspectIndex >= 0 && config.inspectIndex < NUM_LEDS) {
        pwrState = targetPower ? PWR_ON : PWR_OFF;
        if (!targetPower) {
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

        if (config.inspectIndex != lastInspectIndex || baseColor != lastSolidColor ||
            targetDim != lastDeskDim || useGammaCorrection != lastUseGamma) {
            lastInspectIndex = config.inspectIndex;
            lastSolidColor = baseColor;
            lastDeskDim = targetDim;
            lastUseGamma = useGammaCorrection;
            FastLED.clear();

            uint8_t r = scale8(baseColor.r, targetDim);
            uint8_t g = scale8(baseColor.g, targetDim);
            uint8_t b = scale8(baseColor.b, targetDim);
            if (useGammaCorrection) {
                leds[config.inspectIndex] = CRGB(pgm_read_byte(&cie1931[r]),
                                                pgm_read_byte(&cie1931[g]),
                                                pgm_read_byte(&cie1931[b]));
            } else {
                leds[config.inspectIndex] = CRGB(r, g, b);
            }
            if (config.inspectIndex == 36 && NUM_LEDS > 36) {
                leds[36] = blend(lastLed36, leds[36], 190);
                lastLed36 = leds[36];
            }
            FastLED.show();
            Serial.printf("[LED Inspect] Solitary Pixel Active: LED #%d | RGB(%d,%d,%d)\n", 
                          config.inspectIndex, config.r, config.g, config.b);
        }
        return;
    }
    lastInspectIndex = -1;

    // Power Transition State Machine Management (1000ms On / 1000ms Off Synchronized to Lamps)
    if (targetPower && (pwrState == PWR_OFF || pwrState == PWR_TURNING_OFF)) {
        pwrState = PWR_TURNING_ON;
        transitionStartTime = now;
        currentAnimMode = onOffAnimation.length() > 0 ? onOffAnimation : "gaussian-ripple";
        transitionDuration = 1000; // Synchronized with 1.0s lamp fade-in
        Serial.printf("[LED Power] Transition ON -> Mode: %s (%u ms)\n", currentAnimMode.c_str(), transitionDuration);
    } else if (!targetPower && (pwrState == PWR_ON || pwrState == PWR_TURNING_ON)) {
        pwrState = PWR_TURNING_OFF;
        transitionStartTime = now;
        for (int i = 0; i < NUM_LEDS; i++) {
            snapshotLeds[i] = leds[i];
        }
        currentAnimMode = onOffAnimation.length() > 0 ? onOffAnimation : "gaussian-ripple";
        transitionDuration = 1000; // Synchronized with 1.0s lamp fade-out
        Serial.printf("[LED Power] Transition OFF -> Mode: %s (%u ms)\n", currentAnimMode.c_str(), transitionDuration);
    }

    if (pwrState == PWR_OFF) {
        if (lastMasterOn || lastDeskPower) {
            FastLED.clear(true);
            lastMasterOn = false;
            lastDeskPower = false;
        }
        return;
    }

    if (pwrState == PWR_TURNING_OFF) {
        float elapsed = (float)(now - transitionStartTime);
        float t = elapsed / (float)transitionDuration;
        if (t >= 1.0f) {
            pwrState = PWR_OFF;
            FastLED.clear(true);
            lastMasterOn = false;
            lastDeskPower = false;
            return;
        }

        renderOffAnimation(t, currentAnimMode, useGammaCorrection);

        // Hardware Silicon Micro-Glitch Stabilization for Pixel 36
        if (NUM_LEDS > 36) {
            leds[36] = blend(lastLed36, leds[36], 190);
            lastLed36 = leds[36];
        }

        FastLED.show();
        return;
    }

    // Active Rendering (PWR_TURNING_ON or PWR_ON)
    float masterFactor = (float)masterBrightness / 100.0f;
    float contrastFactor = 1.0f + ((float)contrastRatio / 100.0f);
    if (contrastFactor < 0.0f) contrastFactor = 0.0f;
    if (contrastFactor > 1.5f) contrastFactor = 1.5f;

    float effective = ((float)config.brightness / 100.0f) * masterFactor * contrastFactor;
    uint8_t targetDim = constrain((int)(effective * 255.0f), 0, 255);
    CRGB baseColor = CRGB(config.r, config.g, config.b);

    bool inTransition = (pwrState == PWR_TURNING_ON);

    // 2. Static Solid Color Mode: Update ONLY on change, transition, or 3s keepalive
    if (config.effect == "solid") {
        bool changed = inTransition || (baseColor != lastSolidColor || targetDim != lastDeskDim || 
                        useGammaCorrection != lastUseGamma ||
                        !lastMasterOn || !lastDeskPower || lastEffect != "solid" || 
                        now - lastSolidPush > 3000);
        if (!changed) {
            return;
        }
        lastSolidColor = baseColor;
        lastDeskDim = targetDim;
        lastUseGamma = useGammaCorrection;
        lastMasterOn = masterOn;
        lastDeskPower = config.power;
        lastEffect = "solid";
        lastSolidPush = now;
        fill_solid(leds, NUM_LEDS, baseColor);
        // Falls through into unified post-processing pass below!
    } else {
        lastEffect = config.effect;
        lastUseGamma = useGammaCorrection;
        lastDeskDim = targetDim;
        lastMasterOn = masterOn;
        lastDeskPower = config.power;

        // Periodic heartbeat to Serial to confirm Core 1 rendering
        static uint32_t lastHeartbeat = 0;
        if (now - lastHeartbeat > 3000) {
            lastHeartbeat = now;
            Serial.printf("[LED Task] Dynamic Rendering Active | Effect: %s | Dim: %d/255 | Step: %u\n", 
                          config.effect.c_str(), targetDim, animationStep);
        }

        // Dynamic Animation Rendering into leds[i] in natural sRGB space
        if (config.hasGradient) {
            effectGradientPhysics(config);
        } else if (config.effect == "test-cycle") {
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
            fill_solid(leds, NUM_LEDS, baseColor);
        }
    }

    // ========================================================================
    // UNIFIED POST-PROCESSING PASS: Dimming, On-Animation, Gamma, Pixel 36 Dampener
    // ========================================================================
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t r = scale8(leds[i].r, targetDim);
        uint8_t g = scale8(leds[i].g, targetDim);
        uint8_t b = scale8(leds[i].b, targetDim);
        leds[i] = CRGB(r, g, b);
    }

    // Apply Turn-On Animation Modulation
    if (pwrState == PWR_TURNING_ON) {
        float elapsed = (float)(now - transitionStartTime);
        float t = elapsed / (float)transitionDuration;
        if (t >= 1.0f) {
            pwrState = PWR_ON;
        } else {
            renderOnAnimation(t, currentAnimMode);
        }
    }

    // CIE 1931 Perceptual Lightness
    if (useGammaCorrection) {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = pgm_read_byte(&cie1931[leds[i].r]);
            leds[i].g = pgm_read_byte(&cie1931[leds[i].g]);
            leds[i].b = pgm_read_byte(&cie1931[leds[i].b]);
        }
    }

    // Hardware Silicon Micro-Glitch Stabilization for Pixel 36
    if (NUM_LEDS > 36) {
        leds[36] = blend(lastLed36, leds[36], 190);
        lastLed36 = leds[36];
    }

    FastLED.show();
}

void LedDriver::effectGradientPhysics(const DeviceDesk &config) {
    float speedNorm = (float)(config.speed > 0 ? config.speed : 45) / 100.0f;
    float turbNorm = (float)(config.intensity > 0 ? config.intensity : 50) / 100.0f;
    float phase = animationStep * (speedNorm * 0.12f);

    for (int i = 0; i < NUM_LEDS; i++) {
        float normX = (float)i / (float)(NUM_LEDS - 1);

        if (config.effect == "soap-bubble") {
            float wave = sinf(normX * 6.28f * 2.0f - phase) * 0.5f +
                         sinf(normX * 6.28f * 4.5f + phase * 0.8f) * turbNorm * 0.4f;
            float sampleT = fmodf(normX + wave * 0.25f + 10.0f, 1.0f);
            if (sampleT < 0.0f) sampleT += 1.0f;
            CRGB base = interpolate4Stops(config.stops, sampleT);
            float r = constrain(base.r + cosf(wave * 3.0f * 3.14159f) * 45.0f * turbNorm, 0.0f, 255.0f);
            float g = constrain(base.g + cosf(wave * 3.0f * 3.14159f - 2.0944f) * 45.0f * turbNorm, 0.0f, 255.0f);
            float b = constrain(base.b + cosf(wave * 3.0f * 3.14159f - 4.1888f) * 45.0f * turbNorm, 0.0f, 255.0f);
            leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
        } else if (config.effect == "wave") {
            float wave = sinf(normX * 6.28f * 1.8f - phase) * 0.15f * (0.5f + turbNorm);
            float sampleT = fmodf(normX - (phase * 0.2f) + wave + 10.0f, 1.0f);
            if (sampleT < 0.0f) sampleT += 1.0f;
            leds[i] = interpolate4Stops(config.stops, sampleT);
        } else if (config.effect == "caustics") {
            float w1 = sinf(normX * 18.0f - phase * 1.8f);
            float curvature = cosf(normX * 18.0f - phase * 1.8f) * 18.0f +
                              cosf(normX * 28.0f + phase * 1.3f) * 28.0f;
            float denom = fabsf(1.0f - 0.016f * turbNorm * curvature);
            if (denom < 0.05f) denom = 0.05f;
            float caustic = fmaxf(0.0f, (1.0f / denom) - 0.75f);
            float sparkle = fminf(180.0f, caustic * 38.0f * (0.4f + turbNorm * 0.8f));
            float sampleT = fmodf(normX - phase * 0.08f + w1 * 0.06f + 10.0f, 1.0f);
            if (sampleT < 0.0f) sampleT += 1.0f;
            CRGB base = interpolate4Stops(config.stops, sampleT);
            float r = constrain(base.r + sparkle * 0.75f, 0.0f, 255.0f);
            float g = constrain(base.g + sparkle * 0.95f, 0.0f, 255.0f);
            float b = constrain(base.b + sparkle * 1.15f, 0.0f, 255.0f);
            leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
        } else if (config.effect == "soliton") {
            float p1 = fmodf(phase * 0.18f, 1.0f);
            if (p1 < 0.0f) p1 += 1.0f;
            float p2 = 1.0f - fmodf(phase * 0.24f, 1.0f);
            if (p2 < 0.0f) p2 += 1.0f;
            float d1 = fabsf(normX - p1);
            if (d1 > 0.5f) d1 = 1.0f - d1;
            float d2 = fabsf(normX - p2);
            if (d2 > 0.5f) d2 = 1.0f - d2;
            float c1 = coshf(d1 * 24.0f);
            float c2 = coshf(d2 * 28.0f);
            float e1 = 1.0f / (c1 * c1);
            float e2 = 1.0f / (c2 * c2);
            float totalEnergy = e1 + e2;
            float sampleT = fmodf(normX + (e1 - e2) * 0.25f * turbNorm + 10.0f, 1.0f);
            if (sampleT < 0.0f) sampleT += 1.0f;
            CRGB base = interpolate4Stops(config.stops, sampleT);
            float boost = totalEnergy * 160.0f * (0.5f + turbNorm * 0.7f);
            float r = constrain(base.r + boost * 0.9f, 0.0f, 255.0f);
            float g = constrain(base.g + boost * 0.6f, 0.0f, 255.0f);
            float b = constrain(base.b + boost * 1.2f, 0.0f, 255.0f);
            leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
        } else if (config.effect == "thermal-plume") {
            float h1 = sinf(normX * 10.0f - phase * 1.6f);
            float h2 = sinf(normX * 22.0f + phase * 2.4f) * 0.5f;
            float h3 = sinf(normX * 42.0f - phase * 4.2f) * 0.25f;
            float fbm = (h1 + h2 + h3) / 1.75f;
            float sampleT = constrain(normX + fbm * 0.22f * turbNorm, 0.0f, 1.0f);
            CRGB base = interpolate4Stops(config.stops, sampleT);
            float thermalGlow = (fbm + 1.0f) * 36.0f * turbNorm;
            float r = constrain(base.r + thermalGlow * 1.2f, 0.0f, 255.0f);
            float g = constrain(base.g + thermalGlow * 0.6f, 0.0f, 255.0f);
            float b = constrain(base.b + thermalGlow * 0.1f, 0.0f, 255.0f);
            leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
        } else if (config.effect == "marangoni") {
            float cell = sinf(normX * 3.14159f * 6.0f);
            float roll = cell * cosf(phase * 1.4f);
            float shear = sinf(normX * 3.14159f * 12.0f + phase * 0.9f) * 0.1f * turbNorm;
            float sampleT = fmodf(normX + roll * 0.16f + shear + 10.0f, 1.0f);
            if (sampleT < 0.0f) sampleT += 1.0f;
            CRGB base = interpolate4Stops(config.stops, sampleT);
            float vortexGlow = fabsf(cell) * 32.0f * turbNorm;
            float r = constrain(base.r + vortexGlow * 0.5f, 0.0f, 255.0f);
            float g = constrain(base.g + vortexGlow * 1.1f, 0.0f, 255.0f);
            float b = constrain(base.b + vortexGlow * 0.9f, 0.0f, 255.0f);
            leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
        } else if (config.effect == "breathe") {
            float pulse = sinf(phase * 1.5f) * 0.35f;
            float sampleT = constrain(0.5f + (normX - 0.5f) * (1.0f + pulse), 0.0f, 1.0f);
            leds[i] = interpolate4Stops(config.stops, sampleT);
        } else { // shimmer or fallback
            float sparkle = sinf(normX * 60.0f + phase * 8.0f) * turbNorm * 35.0f;
            float sampleT = fmodf(normX - (phase * 0.15f) + 10.0f, 1.0f);
            if (sampleT < 0.0f) sampleT += 1.0f;
            CRGB base = interpolate4Stops(config.stops, sampleT);
            float r = constrain(base.r + sparkle, 0.0f, 255.0f);
            float g = constrain(base.g + sparkle * 0.8f, 0.0f, 255.0f);
            float b = constrain(base.b + sparkle * 1.2f, 0.0f, 255.0f);
            leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
        }
    }
}

void LedDriver::effectSolid(CRGB targetColor, uint8_t brightness) {
    fill_solid(leds, NUM_LEDS, targetColor);
}

void LedDriver::effectBreathe(CRGB baseColor, uint8_t brightness, uint8_t speed) {
    uint8_t breathWave = sin8(animationStep * (speed / 15 + 1));
    uint8_t breathFactor = map(breathWave, 0, 255, 60, 255);
    CRGB c = baseColor;
    c.nscale8(breathFactor);
    fill_solid(leds, NUM_LEDS, c);
}

void LedDriver::effectCyberpulse(uint8_t brightness, uint8_t speed, uint8_t intensity) {
    CRGB magenta = CRGB(210, 50, 130);
    CRGB cyan = CRGB(0, 190, 210);

    uint8_t waveOffset = animationStep * (speed / 12 + 1);
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t blendVal = sin8((i * (intensity / 10 + 1)) + waveOffset);
        leds[i] = blend(magenta, cyan, blendVal);
    }
}

void LedDriver::effectSunsetWave(uint8_t brightness, uint8_t speed) {
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
    uint8_t waveOffset = animationStep * (speed / 14 + 1);

    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t wave = sin8((i * 4) + waveOffset);
        leds[i] = blend(baseColor, CRGB::White, wave / 3);
    }
}

void LedDriver::effectShimmer(CRGB baseColor, uint8_t brightness, uint8_t speed) {
    fill_solid(leds, NUM_LEDS, baseColor);

    uint8_t sparkChance = map(speed, 0, 100, 15, 60);
    for (int i = 0; i < NUM_LEDS; i++) {
        if (random8() < sparkChance) {
            leds[i] += CRGB(70, 70, 70);
        }
    }
}

void LedDriver::effectCaustics(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity) {
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
            leds[idx] += CHSV(24, 200, mag);
        }
    }
}

void LedDriver::effectThermalPlume(CRGB baseColor, uint8_t brightness, uint8_t speed, uint8_t intensity) {
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
