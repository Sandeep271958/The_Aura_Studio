# AURA // ESP32 Smart Lighting Room Hub

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

## 🚀 Key Features

- **Self-Contained ESP32 Smart Hub:** Runs its own responsive web server and WebSocket broadcaster. No external Raspberry Pi, Zigbee hub, or Home Assistant box needed.
- **100% Local WiZ UDP Protocol:** Directly commands Philips WiZ bulbs over UDP port `38899` with instantaneous local execution (<10ms). Operates even when your home internet is offline.
- **Interactive 2.5D Spatial Room Visualizer:** A real-time reactive SVG/CSS room map that mirrors your fixtures' live colors, animations, lumens, and contrast ratio.
- **FreeRTOS Dual-Core Multitasking:**
  - **Core 0:** Wi-Fi stack, LittleFS web server, WebSockets, WiZ discovery & UDP broadcasting.
  - **Core 1:** FastLED hardware engine rendering smooth 60 FPS WS2812B animations without stutter.
- **Curated Atmospheric Mood Scenes:**
  - ⚡ **Cyberpunk Night:** Neon magenta desk pulse & deep cobalt corner wash.
  - ☕ **Warm Minimalist:** 2400K candlelight corners & soft amber breathing desk glow.
  - 🎯 **Deep Focus:** 4500K crisp neutral bias & dimmed 15% ambient surround.
  - 🌅 **Sunset Horizon:** Coral/peach wave with warm golden corner accents.
  - 🌌 **Aurora Drift:** Undulating emerald & violet waves.
  - 🎬 **Midnight Cinema:** Ultra-dim 1800K deep ember with anti-glare task bias.

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
├── dashboard/                   # Standalone Web Dashboard (Browser & Mobile Ready)
│   ├── index.html               # Semantic HTML5 with 2.5D Room Architecture
│   ├── style.css                # Obsidian glassmorphism design system & accessible sliders
│   └── app.js                   # WebSocket engine & dual-mode simulator
├── firmware/                    # ESP32 FreeRTOS Firmware
│   ├── platformio.ini           # PlatformIO configuration
│   ├── data/                    # LittleFS assets (automatically served by ESP32)
│   │   ├── index.html
│   │   ├── style.css
│   │   └── app.js
│   └── src/
│       ├── Config.h             # Pins, Wi-Fi credentials, bulb IPs, defaults
│       ├── LedDriver.h / .cpp   # FastLED 60FPS non-blocking animation driver
│       ├── WizController.h / .cpp # WiZ local UDP driver & discovery parser
│       ├── WebServerManager.h / .cpp # AsyncWebServer & WebSocket sync
│       └── main.cpp             # FreeRTOS dual-core initialization
└── README.md                    # Documentation & setup guide
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
2. The dashboard automatically detects offline mode and starts the **Interactive Room Simulator**, allowing you to preview presets, adjust sliders, and test WiZ network discovery.

---

## 📄 License
MIT License. Created by [Sandeep271958](https://github.com/Sandeep271958).
