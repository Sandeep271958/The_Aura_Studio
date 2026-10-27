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

### 4. 🎨 Gradient Colorway & Dynamic Fluid Lab (Real Wave Physics)
- **Real-World Fluid Wave Physics & Thin-Film Soap Bubble Interference:**
  - Models physical light interference on microscopic fluid films ($\delta = \frac{4\pi}{\lambda} d \cos\theta$), generating organic, pearlescent cyan-magenta-lavender-gold swirls.
  - **Live 60 FPS HTML5 Canvas Fluid Simulation:** Dual-zone visualizer rendering both the continuous multi-harmonic wave ribbon and the 120 individual physical WS2812B LED beads in real time.
- **4-Stop Linear Colorway Designer:**
  - Precision color stops at 0%, 33%, 67%, and 100% with real-time linear CSS preview bar and canvas synchronization.
- **4 Fluid Dynamics & Motion Physics Profiles:**
  - 🫧 **Soap Bubble (Interference):** Multi-harmonic wave displacement with chromatic phase shifting.
  - 🌊 **Directional Wave:** Smooth laminar fluid flow with sinusoidal turbulence dispersion.
  - 💨 **Harmonic Breathing:** Symmetric lung-like expansion and contraction from center bias.
  - ✨ **Prismatic Shimmer:** Micro-refraction dispersion sparkle overlaid on flowing color gradients.
- **Physics Controls:**
  - **Flow Velocity Slider (10–100%):** From gentle deep calm to rapid fluid river flows.
  - **Wave Turbulence Slider (10–100%):** Smooth laminar flow to organic vortex plasma dispersion.
- **10 Luxury Curated Designer Palettes:**
  1. 🫧 *Soap Bubble Iridescence* (`#a5f3fc`, `#f472b6`, `#c084fc`, `#fef08a`)
  2. 🌊 *Bioluminescent Deep Sea* (`#00f2fe`, `#1d4ed8`, `#0b0f19`, `#0d9488`)
  3. 💎 *Opal Prism* (`#ffffff`, `#fde047`, `#22d3ee`, `#e879f9`)
  4. 🌌 *Northern Lights (Aurora)* (`#10b981`, `#06b6d4`, `#8b5cf6`, `#0f172a`)
  5. 🔥 *Solar Flare & Plasma* (`#fef9c3`, `#f59e0b`, `#dc2626`, `#4c1d95`)
  6. 🌸 *Kyoto Cherry Blossom* (`#fbcfe8`, `#a78bfa`, `#fbbf24`, `#1e1b4b`)
  7. ⚡ *Cyberpunk Synthwave* (`#ff007f`, `#7928ca`, `#00f2fe`, `#0a0a14`)
  8. 🏜️ *Mojave Sunset* (`#f59e0b`, `#f43f5e`, `#6b21a8`, `#1e1b4b`)
  9. ☕ *Amber Candle & Hearth* (`#ea580c`, `#f97316`, `#fbbf24`, `#292524`)
  10. 🌴 *Miami Sunset Vice* (`#ff1493`, `#00e5ff`, `#4a00e0`, `#ff007f`)
- **Corner WiZ Bulbs Integration Architecture:**
  - **Dynamic Spatial Sync:** Left and Right WiZ bulbs sample the fluid wave boundary edges, expanding the desk strip across the room.
  - **Architectural Warm White (Gallery Contrast):** WiZ bulbs cast pure CCT white (*2000K Candlelight*, *2400K Warm Minimalist*, *2700K Soft Living*, or *4000K Studio Focus Neutral*), framing the dynamic desk strip with high-end museum gallery ambiance.
  - **Custom Ambient Bulbs:** Independent color picker for personalized corner bulb wash.

### 5. ⏳ Focus & Rest Rhythm (Smart Pomodoro Lighting)
- **Focus Interval (25 min):** 4500K crisp neutral white task bias; corner WiZ bulbs dim to 15% to eliminate room distractions.
- **Rest Interval (5 min):** Room blooms into a soft, warm 2400K golden amber wash with slow breathing pulses to cue relaxation.

### 6. 🎬 Cinematic Transition Timing & Perceptual Dimming
- Configurable fade transitions: *Instant (0s)*, *Snappy (0.4s)*, *Smooth (0.8s)*, *Cinema (2.5s)*, and *Sunset Drift (8.0s)*.
- **CIE 1931 Perceptual Gamma Correction** ($Y = X^{2.2}$) ensuring natural, logarithmic dimming without harsh steps.

### 7. 🏛️ World-Class Architectural Atmosphere Presets
Curated scenes inspired by luxury 5-star hospitality design (Lutron Ketra, Aman Resorts Kyoto, Nordic wellness spas):
1. ⚡ **Cyberpunk Night:** Neon Magenta desk (`#ec4899`) with Deep Cobalt corners (`#1e40af`) for immersive synthwave gaming.
2. ☕ **Warm Minimalist:** 2400K Candlelight corners with soft amber desk breathe effect (`#f59e0b`).
3. 🎯 **Deep Focus:** 4500K crisp neutral white bias with 15% dimmed ambient corners for distraction-free sprints.
4. 🌅 **Sunset Horizon:** Golden peach & sunset wave (`#f43f5e`) with warm 2700K gold corners (`#fb923c`).
5. 🌌 **Aurora Drift:** Emerald & violet fluid flow (`#10b981`) with jade ambient bounce (`#8b5cf6`).
6. 🎬 **Midnight Cinema:** Ultra-dim 1800K ember with anti-glare monitor bias for cinematic viewing.
7. 🌸 **Aman Kyoto Zen:** Serene Japanese ryokan tranquility. 2200K Sakura dawn wave (`#fbcfe8`) paired with 2000K intimate candlelight corners (`#ff8a12`).
8. 🏛️ **Grand Welcome (Lutron Ketra):** High-end hotel arrival scene. Flattering 2800K incandescent ambient baseline with rich warm amber-gold wall wash (`#ffc078`).
9. 🌿 **Deep Sanctuary Spa:** Nordic wellness recovery. Restorative aquamarine bio-luminescent desk glow (`#0d9488`) with 2400K warm decompression wash (`#ff9b3d`).
10. 🌊 **Oceanic Abyss:** Bioluminescent deep-sea cyan pulse (`#00f2fe`) transitioning into deep midnight cobalt corners (`#0b192c`).

### 8. 📱 MacBook & iPhone 16 Boundary-Clean Responsive Architecture
- **MacBook Pro & Air Display Boundary Guarantee:** Designed with fluid flexbox containment, zero-overflow headers, auto-scrolling suite navigation tabs (`scrollbar-width: none`), and intelligent responsive collapsing (`@media (max-width: 1440px)` and `@media (max-width: 1180px)`). Prevents header options and the master **ROOM** power toggle from ever overflowing or clipping on MacBook Air 13" (1280px/1312px) or MacBook Pro 14"/16" viewports.
- **Focus Rhythm Countdown Circle Containment:** Generous 260px circular ring with radial glassmorphism and auto-wrapping pill badges (`FOCUS SPRINT • 4500K`, `REST INTERVAL • 2400K`). Completely contains countdown digits and biological lighting status tags with zero border clipping.
- **Dynamic Island & Safe Area Integration:** Built with `viewport-fit=cover` and dynamic CSS variables (`env(safe-area-inset-top)`, `env(safe-area-inset-bottom)`) ensuring edge-to-edge immersion without clipping behind the iPhone 16 Dynamic Island or home indicator.
- **Ergonomic Obsidian Glass Bottom Dock:** On screens &le; 768px, navigation seamlessly transforms into an iOS-style fixed bottom tab bar for effortless single-thumb access to all 6 feature suites.
- **Apple HIG Touch Ergonomics:** Minimum 44&times;44 pt touch targets, enlarged touch slider thumbs (`26px`), and `touch-action: pan-y` preventing horizontal scrubbing from triggering scroll locks.
- **Tactile Haptic Feedback:** Physical tactile clicks (`navigator.vibrate`) on preset activations, fixture toggles, mode switches, and slider release.
- **Standalone PWA:** Tap "Share" &rarr; "Add to Home Screen" in iOS Safari for a standalone app with custom high-res obsidian-cyan app icons.

### 9. 🌈 Continuous Rainbow Hue Bar & Curated Top Color Matrix
- **360&deg; Continuous Spectrum Scrubbing:** Features a smooth rainbow-gradient Hue Bar (`0&deg;` to `360&deg;`) on the Desk Strip and both WiZ bulbs for instant, fluid color selection without needing to open OS color pickers.
- **Top Color Choices Matrix Grid:** Curated 12-color smart lighting palette with glowing halo swatches:
  - *Primary & Neon:* Cyber Magenta (`#ec4899`), Electric Cyan (`#00f2fe`), Royal Violet (`#8a2be2`), Deep Cobalt (`#1e40af`), Emerald Jade (`#10b981`), Vivid Amber (`#f59e0b`), Crimson Ruby (`#ef4444`).
  - *Atmospheric & Natural:* Sunset Coral (`#fb7185`), Sky Cerulean (`#0284c7`), Candlelight 2000K (`#ff9329`), Soft Warm 2700K (`#ffa757`), Studio Daylight White (`#ffffff`).
- **Bi-Directional Color Synchronization:** Live dual readout (`#EC4899 • 330° Hue`), auto-positioning Hue slider thumbs, active selection rings, and native color wheel integration.

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
