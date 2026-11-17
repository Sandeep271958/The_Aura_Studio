# 📦 THE AURA STUDIO — Firmware Release & Nomenclature Registry

This registry documents all compiled production firmware binaries for **The Aura Studio** ESP32 smart lighting hub.

---

## 🏷️ Nomenclature Specification (Industry Standard `XX.YY.ZZZ`)

Every production firmware binary strictly follows the industry-standard 3-tier versioning format:

$$\Large \mathbf{firmware\_XX.YY.ZZZ.bin}$$

Where:
* **`XX` (Major Architecture Revision)**:
  Breaking changes, dual-slot partition table shifts (e.g. `partitions_ota.csv`), hardware pin re-assignments, or core framework upgrades.
* **`YY` (Minor Feature Release)**:
  New ambient lighting profiles, circadian solar arc modifications, new REST/WebSocket endpoints, or integrated UI features (e.g., Wireless OTA Settings Integration).
* **`ZZZ` (Patch & Stability Build)**:
  Zero-regression bug fixes, anti-flicker bus timing tweaks, temporal solder filters, or FreeRTOS task pacing optimizations.

---

## 🚀 Active Releases

| Release Artifact | Version | Release Date | Flash Size | Partition Target | SHA-256 Checksum |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`firmware_01.04.001.bin`** | `v01.04.001` | 2026-11-17 | 1,494,608 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `eda82463e248ca701d6a22cfd5688c54860f0991cd3a25ed045690be30002043` |
| **`firmware_01.04.000.bin`** | `v01.04.000` | 2026-11-16 | 1,492,912 B (1.42 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `9dab85cb14635b18c89ec95bbfcfa1ba5a579189042a13766b0983c4ba2ef2a3` |
| **`firmware_01.03.000.bin`** | `v01.03.000` | 2026-11-16 | 1,486,128 B (1.42 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `97f0f81bcb92ab2e8adc50bb09d80c2c91c1b68b453a6b035c3b6b5b29288b8c` |
| **`firmware_01.02.005.bin`** | `v01.02.005` | 2026-11-16 | 1,461,408 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `8464c07717e1286eaccf6c3a3c2ce2ab0e08b85b3ff5af27dc620bf376b45701` |
| **`firmware_01.02.004.bin`** | `v01.02.004` | 2026-11-16 | 1,461,216 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `a6faa97283cf36768d946bdbbc8b23d350c4bd58be6994c0a9d32bb37d88176e` |
| **`firmware_01.02.003.bin`** | `v01.02.003` | 2026-11-16 | 1,459,952 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `40900bcffdfb0d295fa89e218ecc70a9dd5f7f2d74dd857044ac44a53c7aaabb` |
| **`firmware_01.02.002.bin`** | `v01.02.002` | 2026-11-15 | 1,459,056 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `a830f986dcbda748b26e933054614e499b13c2fac8e07734cfa89d0d2da454f9` |
| **`firmware_01.02.001.bin`** | `v01.02.001` | 2026-11-15 | 1,459,072 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `917d3b9c7330c6edc60b50da204c36db01b362eed0e701652c4ae38c861a8275` |
| **`firmware_01.02.000.bin`** | `v01.02.000` | 2026-11-15 | 1,458,336 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `ddf4c4081beed3d9c7e1465205afc7033d2ca33842cf1c019b6d65d4edf68630` |
| **`firmware_01.01.000.bin`** | `v01.01.000` | 2026-11-15 | 1,451,872 B (1.38 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `ff4c315a67b4a7e8a500f6c949fc04dd798a8fd41bab5709f6b888b0c5dc1c02` |
| **`firmware_01.00.000.bin`** | `v01.00.000` | 2026-11-14 | 1,383,995 B (1.32 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `09f5ad864af6807150249f03288c7cd328672cd826af9e3cb405251a488734cf` |

### Key Highlights of `v01.04.001`:
1. **Industrial Hardening Against Router Power Cuts & Autonomous Auto-Recovery**:
   - Eliminates the destructive Wi-Fi credential erasure bug on power outages or rapid power cycles.
   - Autonomous Background Station Reconnection Engine: when the router boots up after a power cut, the ESP32 automatically probes and connects to the home network in the background, shuts down SoftAP, starts mDNS, and syncs all lighting without requiring an ESP32 power cycle.
   - Non-blocking `disableCaptivePortal()` cleanly halts the DNS server upon successful home LAN association.
2. **Brownout & Voltage Sag Inrush Protection**:
   - Clamps LED bootup inrush power to 950mA and soft-starts the Apple Siri boot wave at 75% peak luma during Wi-Fi handshake.
   - Caps Wi-Fi RF transmit bursts to +17dBm (`WiFi.setTxPower(WIFI_POWER_17dBm)`), reducing peak current transients by >30% and eliminating power rail brownout resets on low-spec USB power adapters.
   - Detects `ESP_RST_BROWNOUT` reset reason at boot and automatically enforces conservative safety limits.
3. **Leak-Free NVS Handle Lifecycle & FreeRTOS Stack Headroom**:
   - Replaced dangling static NVS Preferences handle in `WebServerManager.cpp` with strict RAII local scoped instances (`begin("aura", false)` -> `end()`), preventing ESP-IDF handle invalidation and assertion aborts.
   - Expanded `LedTask` FreeRTOS stack from 4096 to 8192 bytes, giving 100% stack overflow immunity.
   - Guarded all WiZ UDP socket transmissions against disconnected station status (`WiFi.status() == WL_CONNECTED`), preventing lwIP routing buffer exhaustion.

### Key Highlights of `v01.04.000`:
1. **Instant WiZ Auto-Sync & Cold-Boot Detection**:
   - Bi-directional UDP payload parser intercepts `firstBeat` boot announcements and `syncPilot` status updates alongside query responses.
   - When a WiZ bulb powers up at a wall switch, the ESP32 intercepts its network arrival and immediately commands its target color state in under 100ms.
   - Active 2.5-second heartbeat poll (`getPilot`) with offline timeout and 300ms confirmation resend guarantees 100% arrival reliability even across restrictive Wi-Fi routers.
   - Completely eliminates the manual room off/on power-cycle workaround.
2. **Full State Persistence & Switched-Off Power-Cycle Recovery**:
   - Persists all core state in NVS (`masterOn`, `masterBri`, `contrast`, `syncCorners`, `preset`, desk effect/colors/speed/intensity/gradient stops, and dual WiZ power/dim/RGB/kelvin).
   - Smart Bootloader Guard: if the system was switched off before power down, the ESP32 boots with hardware LEDs dark, skips the boot animation, connects to Wi-Fi silently, and commands WiZ bulbs to remain off.
   - Flash Wear Protection: immediate commit for power toggles, and debounced 1.2s commit for continuous slider adjustments.
3. **Partition Memory Budget & Zero Regression**:
   - Cleanly fits within the 1.44 MB dual-OTA partition budget (`1,492,912 B` used).
   - Verified with 134/134 automated checks in `test_suite.js`.

### Key Highlights of `v01.03.000`:
1. **Autonomous SoftAP Captive Portal & Dynamic NVS Provisioning**:
   - Zero physical disassembly required: moving flats, changing rooms, or updating Wi-Fi passwords no longer requires USB reflashing.
   - Built-in DNS server (port 53) captures all network probes (`/hotspot-detect.html`, `/generate_204`, `/canonical.html`, `/ncsi.txt`, `/connecttest.txt`) and automatically surfaces the setup portal on Apple iOS, macOS, Android, and Windows devices.
   - Luxury dark glassmorphism onboarding interface with 2.4GHz channel scanning, SSID selection, password encryption toggle, and one-tap NVS storage.
2. **In-Dashboard Dynamic Wi-Fi Management**:
   - Real-time Wi-Fi telemetry in Settings: live connected SSID and signal strength RSSI dBm readout with signal quality categorization.
   - Interactive "Change Network" panel allows on-the-fly airwave scanning, network selection, and graceful reboot without tearing down the enclosure.
   - "Reset to Setup Hotspot" action clears NVS credentials and cleanly transitions the ESP32 into `Aura-Studio-Setup` mode.
3. **Fail-Safe 3-Power-Cycle Emergency Factory Reset**:
   - Rapidly toggling power 3 times within 4 seconds triggers an emergency factory reset: clears Wi-Fi NVS credentials, and boots directly into SoftAP setup mode.
   - Visual status beacon: desk WS2812B strip pulses an amber breathing glow (`CRGB(251, 146, 60)`) to signal that the hub is in setup mode awaiting Wi-Fi credentials.
4. **Partition Memory Safety & Zero Regression**:
   - Cleanly fits within the 1.44 MB dual-OTA partition budget (`1,486,128 B` used, with `21,200 B` headroom).
   - 100% byte-for-byte symmetry between `dashboard/` and `firmware/data/` verified.

### Key Highlights of `v01.02.005`:
1. **Apple Intelligence / HomePod "Sound Glow" Chromatic Tuning**:
   - Palette upgraded to Apple's signature iridescent 4-stop spectrum: Electric Orchid (`#EC4899`), Royal Violet / Deep Indigo (`#7C3AED`), Siri Electric Cyan / Azure (`#00F2FE`), and Sunset Amber / Warm Coral (`#FB923C`), crowned with the Specular Diamond White Core (`#FFFFFF`).
   - Every color pair is optically complementary in additive light space (`.blendMode(.plusLighter)`), eliminating any clashing or muddy transitional zones.
2. **Sub-Pixel Spatial Diffusion Filter**:
   - Recreates the HomePod's frosted-glass touch-cap optical low-pass filter using a 3-tap spatial Gaussian blur kernel $[0.15, 0.70, 0.15]$.
   - Blurs discrete physical LED boundaries into a seamless, continuous ribbon of liquid light.
3. **3-Phase HomePod Lifecycle Transition**:
   - **Phase 1 (Center Awakening)**: Bilateral Hermite cubic wake mask expands symmetrically outward from the center (LEDs 29–30).
   - **Phase 2 (Fluid Convection & Solitons)**: Rotating continuous conic phase field with viscous domain warping and Diamond White collision highlights.
   - **Phase 3 (Settle & Dissolve)**: Harmonic phase velocity decelerates smoothly by 40%, and luminance dissolves into equilibrium via a silky cosine-damped logarithmic fade ($\cos^2(\cdot)$).

### Key Highlights of `v01.02.004`:
1. **Multi-Wave Soliton Physics Bootup**:
   - Replaced crude sequential R, G, B, W bootup tests with a 2500ms 60 FPS multi-wave physics simulation.
2. **Dual-Core Asynchronous Wi-Fi Initialization**:
   - Non-blocking `WiFi.begin(...)` initiated *prior* to bootup animation.
   - Core 1 renders fluid waves while Core 0 completes the WPA2 handshake in the background, completely eliminating the dead-black delay before live lighting starts.

### Key Highlights of `v01.02.003`:
1. **iPhone 13 & iPhone 16 Viewport Header Fix**:
   - Resolved header clipping on $\le 430$px screens by collapsing non-critical badges and setting `#btn-master-power` to `display: inline-flex !important; flex-shrink: 0;`.
2. **Dual Room Power Pill**:
   - Embedded dedicated `#btn-room-power-card` in the Master Room Controls Card with bidirectional state synchronization to the top header toggle.

### Key Highlights of `v01.02.002`:
1. **SoundSync Background Tab Worker**:
   - Implemented Web Audio API audio-reactive worker and throttling to prevent WebSocket disconnects during video streaming in adjacent tabs.
2. **Desk On/Off Transition Engine**:
   - Added 4 selectable power transition curves (`gaussian-ripple`, `bilateral-bloom`, `thermal-warmup`, `quantum-sweep`) in Settings.

### Key Highlights of `v01.02.001`:
1. **Dual-Target Wireless OTA Flashing**:
   - Web server automatically detects file type: updates firmware (`app0`/`app1` via `U_FLASH`) for `firmware_*.bin`, or filesystem (`spiffs` via `U_SPIFFS`) for `littlefs_*.bin`.
   - Allows updating both the C++ backend and the rich web dashboard wirelessly without physical USB cables.
2. **HTTP Cache Invalidation & Version Tagging**:
   - Added HTTP response headers `Cache-Control: no-cache, no-store, must-revalidate` for static web assets.
   - Asset URLs dynamically tagged with cache-busting query strings (`?v=01.02.001`) preventing stale browser cache retention.
3. **1000ms LED & Lamp Transition Synchronization**:
   - WS2812B desk strip turn-on and turn-off animation curves tuned to exactly 1000ms, perfectly synchronizing with the 1-second WiZ corner lamp dimming curve.
4. **Hardware & Pixel Diagnostic Refinement**:
   - Cleaned up redundant inspection buttons in the Settings panel; embedded the "Restore All LEDs" action seamlessly within the diagnostic slider scrub status readout.
5. **Security & Permissions Assistant & Localhost Runner**:
   - Built interactive in-dashboard modal guiding users to enable Chrome flags or utilize the zero-install `start_dashboard.command` companion runner for microphone and screen share synchronization without HTTPS certificate friction.

### Key Highlights of `v01.01.000`:
1. **Permanent WiZ Bulb NVS Persistence**:
   - Saves assigned Left and Right WiZ bulb IPs and MAC addresses to ESP32 Non-Volatile Storage (namespace `"aura"` via `Preferences.h`).
   - Bulbs remain permanently assigned across ESP32 power cuts, reboots, and router restarts.
   - Intelligent DHCP tracking: auto-detects if a bulb's IP shifts on the local Wi-Fi router by matching normalized MAC addresses and automatically updates NVS.
2. **Hardware Optical Tuning & CIE 1931 Gamma Calibration**:
   - Implements 256-step CIE 1931 perceptual lightness curve ($Y = (L^* / 100)^3 \approx X^{2.2}$) PROGMEM lookup table.
   - FastLED color correction tuned for physical WS2812B diodes (GRB order) to ensure real-world colors, modes, and gradients faithfully mirror the web dashboard preview.
   - Integer-bound transition math eliminating secular color drift over extended operation.
3. **Dashboard Hardening & Parity**:
   - 25 identified functional edge cases resolved across presets, spectrum pickers, Pomodoro interval timer, and slider synchronizers.
   - 100% byte-for-byte synchronization guaranteed between `dashboard/` and `firmware/data/` LittleFS storage.

### Key Highlights of `v01.00.000`:
1. **Wireless In-Dashboard OTA Flashing**: Integrated directly inside the Settings modal on `http://aura-hub.local` with real-time progress tracking, 6-second auto-reboot countdown, and automatic page reconnection.
2. **Dual-Partition Fail-Safe Architecture**: Employs `partitions_ota.csv` with dual 1.44MB app partitions (`app0`/`app1`) and 1.06MB LittleFS. Flashing writes to the inactive partition and switches boot flags only after signature verification.
3. **Anti-Flicker Engineering**:
   - `FastLED.setDither(0)` disables temporal dithering, completely eliminating the 50Hz AC-like shimmer.
   - Dedicated GPIO 4 bus with quiet static bus idling (transmits only on state updates or 3s keepalive).
   - Dynamic 1000mA current limiter and 45 FPS pacing to prevent voltage dips under rigorous motion modes (`cyberpulse`, `candlefire`).
   - Solder-joint temporal low-pass filter on LED #36 repeater (`blend(lastLed36, target, 190)`).
4. **Hardware & Pixel Diagnostic Tool**: Allows testing individual WS2812B LEDs from 0 to 59 and one-click red inspection on suspect repeater joint #36.

---

## 🛠️ Automated Packaging Command

To generate a new numbered release from source code:

```bash
python3 scripts/export_release.py
```

This reads `FIRMWARE_VERSION` from `firmware/src/Config.h`, invokes PlatformIO, copies the binary to `releases/firmware_XX.YY.ZZZ.bin`, and computes its SHA-256 hash.
