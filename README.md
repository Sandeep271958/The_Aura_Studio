# AURA // Luxury Smart Lighting Architecture

> **A production-grade, local smart lighting hub and spatial controller designed for dual Philips WiZ corner bulbs and an addressable WS2812B desktop LED strip.**
> **Zero cloud dependency. Under 10ms local latency. Built with FreeRTOS dual-core firmware and an obsidian glassmorphism spatial dashboard.**

---

## 🌟 The Concept: Layered Room Lighting Architecture

Most room lighting feels flat and jarring because standard RGB strips either overpower the room or fail to integrate with ambient wall lights. **AURA** treats your room as a multi-layered lighting canvas:

```
┌────────────────────────────────────────────────────────┐
│  [Corner WiZ Bulb 1]              [Corner WiZ Bulb 2]  │  <- Layer 1: Ambient Wall Fill
│         \                                /             │     (Bounces off walls, soft fill)
│          \                              /              │
│           ▼                            ▼               │
│                  ┌──────────────────┐                  │
│                  │  Monitor Display │                  │
│                  └────────┬─────────┘                  │
│                           ▼                            │
│              [ WS2812B Bias / Backlight ]              │  <- Layer 2: Contrast & Eye Bias
│             ══════════════════════════════             │     (Rich gradient behind screen)
│                    [ Desk Surface ]                    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

1. **Layer 1: Ambient Fill (2&times; Philips WiZ Corner Bulbs)**
   - Bathes walls in diffused, low-intensity light. Eliminates deep shadows in the room's periphery.
2. **Layer 2: Contrast & Bias Glow (Desk WS2812B Strip)**
   - Backlight behind your monitor/desk to soften screen contrast, banishing eye fatigue during late-night work or gaming.
3. **The Layer Contrast Slider**:
   - Dynamically shifts the illumination balance between ambient wall bounce and direct task bias without altering perceived overall brightness.

---

## 💎 Flagship High-End Feature Suites

AURA incorporates the most advanced capabilities found across luxury lighting ecosystems (**Lutron Ketra**, **Philips Hue Entertainment**, **Govee DreamView**, **Nanoleaf Rhythm**, and **Apple HomeKit Adaptive Lighting**):

### 1. ☀️ Circadian Solar Rhythm (Human Centric Lighting)
- Synchronizes your room's color temperature and lux to the biological 24-hour solar cycle.
- Automatically transitions from **2200K warm dawn** &rarr; **6500K zenith daylight focus** &rarr; **2700K golden hour** &rarr; **1800K deep melatonin-preserving amber** at night.
- Interactive 24-hour solar arc timeline with live sun altitude tracker and manual preview scrubber.

### 2. 🖥️ Spatial Ambilight Real-Time Screen Sync
- Real-time 30 FPS display capture using native browser **WebRTC Screen Capture API** (`getDisplayMedia`) with zero companion desktop apps required.
- Samples 3 distinct spatial zones:
  - **Left Screen Edge** &rarr; Dispatched to Left Corner WiZ Bulb.
  - **Center & Bottom** &rarr; Dispatched to Desk WS2812B Bias Strip.
  - **Right Screen Edge** &rarr; Dispatched to Right Corner WiZ Bulb.
- Features Exponential Moving Average (EMA) color smoothing, saturation boost, and an anti-fatigue black floor.

### 3. 🎵 SoundSync Audio Reactive Visualizer
- Hardware FFT frequency analysis powered by the **Web Audio API**.
- Sub-bass frequencies (20–150 Hz) drive rhythmic room pulses across corner WiZ bulbs, while vocal mids and percussion ripple fluid waves across the desk strip.

### 4. 🎨 Gradient Colorway Studio
- Multi-stop linear colorway designer for the 120 addressable WS2812B LEDs.
- Create custom 3-stop fluid gradients or pick from luxury curated palettes (*Miami Vice*, *Solar Flare*, *Emerald Tide*, *Tokyo Twilight*, *Nordic Deep*).

### 5. ⏳ Focus & Rest Rhythm (Smart Pomodoro Lighting)
- **Focus Interval (25 min):** 4500K crisp neutral white task bias; corner WiZ bulbs dim to 15% to eliminate room distractions.
- **Rest Interval (5 min):** Room blooms into a soft, warm 2400K golden amber wash with slow breathing pulses to cue relaxation.

### 6. 🎬 Cinematic Transition Timing & Perceptual Dimming
- Configurable fade transitions: *Instant (0s)*, *Snappy (0.4s)*, *Smooth (0.8s)*, *Cinema (2.5s)*, and *Sunset Drift (8.0s)*.
- **CIE 1931 Perceptual Gamma Correction** ($Y = X^{2.2}$) ensuring natural, logarithmic dimming without harsh steps.

---

## 🛠️ Hardware Wiring & Schematic

```
           +5V External Power Supply (e.g. 5V 4A)
          ┌───────────────────┴───────────────────┐
          │ (+)                                 (-)
          │  │                                   │
          │  ├───[1000µF, 6.3V+] Capacitor───────┤
          ▼  ▼                                   ▼
        ┌────────┐                            ┌────────┐
        │  +5V   │                            │  GND   │
        │ WS2812B│                            │ WS2812B│
        └────┬───┘                            └────┬───┘
             │                                     │
   Data In ──┼───[330Ω Resistor]───┐               │
             │                     │               │
             ▼                     ▼               ▼
           [ 5V/VIN               GPIO 16         GND ]
           [               ESP32 (NodeMCU)            ]
```

### Power Calculation Formula
$$\text{Max Current (A)} \approx (\text{LED Count} \times 0.05\text{A}) + 0.5\text{A (ESP32)}$$
* For **120 LEDs** (approx. 2 meters of 60 LEDs/m strip): $120 \times 0.05 + 0.5 \approx \mathbf{6.5\text{A}}$ at 100% full white.
* FastLED includes a software safety power limiter configured at `MAX_POWER_MILLIAMPS = 3000` (3A).

---

## 📂 Project Structure

```
SmartLight/
├── dashboard/                     # Standalone Web Dashboard (Browser & Mobile PWA)
│   ├── index.html                 # 6 Feature Suites & 2.5D Room Architecture
│   ├── style.css                  # Obsidian Glassmorphic Design System & Suite Styling
│   └── app.js                     # Circadian, Ambilight, SoundSync & WebSocket Engine
├── firmware/                      # ESP32 FreeRTOS Firmware
│   ├── platformio.ini             # PlatformIO configuration
│   ├── data/                      # LittleFS assets (automatically served by ESP32)
│   │   ├── index.html
│   │   ├── style.css
│   │   └── app.js
│   └── src/
│       ├── Config.h               # Pins, Wi-Fi credentials, bulb IPs, defaults
│       ├── LedDriver.h / .cpp     # FastLED 60FPS non-blocking animation driver
│       ├── WizController.h / .cpp # WiZ local UDP driver & discovery parser
│       ├── WebServerManager.h / .cpp # AsyncWebServer & WebSocket sync
│       └── main.cpp               # FreeRTOS dual-core initialization
└── README.md                      # Documentation & setup guide
```

---

## ⚡ Quick Start: Flashing the ESP32

### Prerequisites
- [VS Code](https://code.visualstudio.com/) with the [PlatformIO IDE Extension](https://platformio.org/).

### 1. Configure Wi-Fi & Hardware
Open [`firmware/src/Config.h`](file:///Users/mouryasaisandeep/Desktop/Projects/SmartLight/firmware/src/Config.h) and set your local credentials:
```cpp
#define DEFAULT_WIFI_SSID       "Your_Home_WiFi"
#define DEFAULT_WIFI_PASSWORD   "Your_WiFi_Password"
#define NUM_LEDS                120  // Your strip length
#define LED_PIN                 16   // GPIO pin connected to strip data
```

### 2. Upload Web Assets to LittleFS
In PlatformIO:
1. Click the PlatformIO Alien icon on the sidebar.
2. Under `esp32dev` &rarr; `Platform`, click **Upload Filesystem Image**.
*(This writes `firmware/data` into the ESP32's flash memory).*

### 3. Flash the Firmware
1. Click **Upload and Monitor**.
2. Note the IP assigned in the Serial Monitor (e.g., `192.168.1.99`) or navigate to:
   ```
   http://aura-hub.local
   ```
3. Tap **"Add to Home Screen"** on your iOS or Android device for a full-screen native app experience.

---

## 🌐 Instant Browser Preview (No Hardware Required)

You can explore and test the dashboard immediately on any computer:
1. Open [`dashboard/index.html`](file:///Users/mouryasaisandeep/Desktop/Projects/SmartLight/dashboard/index.html) in any modern browser.
2. The dashboard automatically detects offline mode and starts the **Interactive Room Simulator**, allowing you to preview presets, adjust sliders, run the Circadian timeline, and test Ambilight and SoundSync.

---

## 📄 License
MIT License. Created by [Sandeep271958](https://github.com/Sandeep271958).
