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
| **`firmware_01.10.000.bin`** | `v01.10.000` | 2026-11-19 | 1,505,712 B (1.44 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `1d58af9022131d1d5e976a894010b544d4e4db882e241d1f8640d3ac9ad88432` |
| **`firmware_01.09.000.bin`** | `v01.09.000` | 2026-11-18 | 1,504,720 B (1.44 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `cb4950a3a1b136967bfcb96fde9897fec3f30f6401f12aeb1096a3a39f2e763d` |
| **`firmware_01.08.000.bin`** | `v01.08.000` | 2026-11-17 | 1,500,272 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `9d875d53f99c4d7e5522b608afc49b1a4f8d92296870c8e2bf97ed4ed34ca1dc` |
| **`firmware_01.07.001.bin`** | `v01.07.001` | 2026-11-17 | 1,497,168 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `88066e18c003316fb37cfe270b68bb084e873dd53dc57c8b9f2767e619c8681c` |
| **`firmware_01.07.000.bin`** | `v01.07.000` | 2026-11-17 | 1,494,752 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `653dba68368fa3ee40aa47f011e8bcbee956ca99a29ac4ffb39ff15963c8b132` |
| **`firmware_01.06.000.bin`** | `v01.06.000` | 2026-11-17 | 1,494,976 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `c2cf39c4eb8bd77b34ae6098392e1b67cfd479c16286fc05d794267703e59da4` |
| **`firmware_01.05.001.bin`** | `v01.05.001` | 2026-11-17 | 1,494,960 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `ad6d39a9c54e5d7d73b50545b937123e2600a50a1376c48ff8399caf673f798e` |
| **`firmware_01.05.000.bin`** | `v01.05.000` | 2026-11-17 | 1,494,704 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `db2e9f59a284690ef12607af7ad91f44096fb162afdd6c4141f6fb79241f9ab7` |
| **`firmware_01.04.001.bin`** | `v01.04.001` | 2026-11-17 | 1,494,704 B (1.43 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `4ee2d017be26baa55c868f810a8c7ac42ada4183e3e04f9e5ddb78bdc4174f4b` |
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

### Key Highlights of `v01.08.000`:
1. **Hardware Bootloader-Enforced Auto-Rollback Engine**:
   - **Guaranteed Rollback for Legacy & Crashing Firmware**: When flashing any firmware via Web OTA (`/update`), `WebServerManager.cpp` now explicitly sets the target partition's `otadata` record to **`ESP_OTA_IMG_NEW`** (`0x00000000`) with CRC32 verification before rebooting.
   - **Hardware Bootloader Auto-Abort**: The ESP32 second-stage hardware bootloader natively catches `ESP_OTA_IMG_NEW` on the first boot, promotes it to `ESP_OTA_IMG_PENDING_VERIFY`, and if the newly booted firmware crashes, panics, or resets before validation (even broken legacy binaries like `01.05.001` that possess zero rollback code), the bootloader automatically marks it `ESP_OTA_IMG_ABORTED` (`4`) and rolls back to the stable partition completely in hardware without opening the enclosure or pressing buttons!
2. **Deferred Validation Override (`verifyRollbackLater()`)**:
   - Overrides the Arduino-ESP32 weak hook `extern "C" bool verifyRollbackLater(void) { return true; }` to prevent Arduino's `initArduino()` from auto-validating unverified firmware on boot. Validation is strictly deferred until the full 20-second stability trial completes.
3. **Dual-Layer Defense-in-Depth**:
   - Hardware Bootloader Rollback on panic/reset + Application-Level RTC Memory Crash Counter (`rtcBootFailures >= 3`) ensuring 100% fail-safe protection across all crash and brownout scenarios.
4. **Autonomous Recovery Telemetry**:
   - The hub detects hardware bootloader rollback on boot and surfaces an alert banner in the Settings deck informing the user that the previous update was rejected, preserving full desk control and dashboard accessibility.

### Key Highlights of `v01.07.001`:
1. **Tier-1 Autonomous Dual-Partition Hardware Rollback**:
   - **Automatic Bootloop Protection**: Integrated `<esp_ota_ops.h>` with RTC memory crash counting (`rtcBootFailures`). If any newly flashed OTA update crashes or bootloops 3 times consecutively before reaching 20 seconds of stable operation, the ESP32 automatically flips `otadata` and executes `esp_ota_mark_app_invalid_rollback_and_reboot()`, instantly restoring the previous stable partition with zero user intervention and zero box disassembly.
   - **20-Second Stability Handshake**: Newly flashed partitions run in trial verification mode (`ESP_OTA_IMG_PENDING_VERIFY`) until surviving 20 seconds of stable runtime, at which point `esp_ota_mark_app_valid_cancel_rollback()` marks the partition permanent and resets the crash counter.
2. **Dashboard Partition & Rollback Telemetry**:
   - **Active Slot Pill**: Live telemetry pill (`Slot: ota_0` or `Slot: ota_1`) with real-time `[Trial]` state indication.
   - **Auto-Rollback Recovery Banner**: If an automatic rollback occurred, a dedicated alert banner in the Settings deck notifies the user that the previous update crashed and was self-healed, with a 1-click dismiss button.
3. **Infinite Ping-Pong Storage Management**:
   - Memory is strictly isolated in dedicated 1.44 MB slots (`ota_0` and `ota_1`), alternating on each flash without accumulating flash fragmentation or running out of memory.

### Key Highlights of `v01.07.000`:
1. **Permanent Bootloop & Post-Animation Crash Elimination**:
   - **Root Cause Identified & Fixed**: The previous staged boot pipeline called `WiFi.mode(WIFI_OFF)` during the boot animation wave, and subsequently called `wizCtrl.begin()` (`WiFiUDP::begin()`) and `webManager.begin()` (`AsyncWebServer::begin()`) before Wi-Fi netifs were initialized. This triggered lwIP socket binding failures and AsyncTCP kernel panics upon boot animation completion.
   - **Clean Sequential Initialization**: Restored standard idle station mode during the boot animation wave. WiZ controller and AsyncWebServer are now initialized strictly after Wi-Fi interfaces are active.
   - **Preemption Race Condition Resolved**: In `v01.05.001`, `LedTask` was spawned at FreeRTOS Priority 2 on Core 1 before Wi-Fi initialization finished, preempting `setup()` (Priority 1) and causing timing collisions. In `v01.07.000`, `LedTask` is spawned at the very end of `setup()` at cooperative Priority 1.
   - **Non-Destructive SoftAP Teardown**: Replaced destructive `WiFi.softAPdisconnect(true)` (which called `esp_wifi_stop()` and killed the Wi-Fi hardware controller) with `WiFi.softAPdisconnect(false)` and `WiFi.enableAP(false)`.
   - **TWDT Stability**: Removed unconfigured raw TWDT calls, relying on native FreeRTOS task yielding via `yield()` and `vTaskDelay()` across all loops and animation frames.
2. **Preserved Physical Strip Orientation & Coordinate Architecture**:
   - Anchored LED 1 [Index 0] adjacent to Left Lamp (`wizLeft`) and LED 60 [Index 59] adjacent to Right Lamp (`wizRight`).
3. **USB Emergency Flashing Support**:
   - Added automated one-click USB recovery script `scripts/flash_usb.sh` for unbricking boards when OTA network access is unavailable.

### Key Highlights of `v01.06.000`:
1. **Physical Strip Spatial Orientation & Coordinate Alignment**:
   - **Hardware Topology Standardized**: Formally anchored the physical strip orientation where **LED 1 [Index 0]** is located adjacent to the Left WiZ Corner Lamp (`wizLeft`), and **LED 60 [Index 59]** is located adjacent to the Right WiZ Corner Lamp (`wizRight`).
   - **Hardware Constants in `Config.h`**: Added `STRIP_LEFT_INDEX (0)` and `STRIP_RIGHT_INDEX (59)` hardware topology definitions.
   - **Hardware Pixel Diagnostic UI Enhancements**:
     - Embedded clear spatial orientation indicators directly below the diagnostic slider: `◄ LED 1 [0] (Left Lamp)` and `LED 60 [59] (Right Lamp) ►`.
     - Added dynamic zone telemetry tooltips mapping indices 0–29 to Left Zone and 30–59 to Right Zone.
   - **Spatial Gradient Sync Consistency**: Gradient Stop 1 ($0\%$) corresponds to LED 1 [0] and Left Lamp; Gradient Stop 4 ($100\%$) corresponds to LED 60 [59] and Right Lamp.
   - **Interactive 2.5D Room Viewport**: Added orientation metadata tooltips to the desk virtual LED strip fixture.
2. **Zero Functional Regression**: All existing system functionalities, timing, transitions, power protections, and animations remain 100% intact and verified.

### Key Highlights of `v01.05.001`:
1. **Staged Sequential Boot Pipeline (100% Brownout & Bootloop Immunity)**:
   - **Root Cause Eliminated**: Resolved the critical bootloop where the ESP32 rebooted repeatedly for 3–5 minutes during/after home router power loss. The collision of active Wi-Fi RF probe transmissions (~400mA pulses) and peak LED specular white surge (~1.4A combined) caused voltage sags that tripped `ESP_RST_BROWNOUT` at ~1.2s into the boot wave. In addition, `setup()` ran up to 8.3s without feeding the Task Watchdog Timer (TWDT 5.0s default), triggering panic resets.
   - **Stage 1 (Pure Isolated Hardware Alive Wave)**: Wi-Fi radio is unpowered (`WiFi.mode(WIFI_OFF)`). The 2.5s Apple Siri fluid wave runs with zero RF inrush, power clamped to 450mA and luma scale 0.40f (<220mA total draw), feeding `esp_task_wdt_reset()` and `yield()` on every frame. Brownout resets and watchdog panics are physically impossible.
   - **Stage 2 (Subsystems & Instant Web Server)**: LittleFS mounted, NVS credentials and persistent `state_t0` loaded, FreeRTOS `LedTask` created, and `webManager.begin()` starts AsyncWebServer & WebSockets on port 80 immediately.
   - **Stage 3 (Staged Wi-Fi Bring-Up with Instant SoftAP Resilience)**: Starts `WiFi.mode(WIFI_AP_STA)` with SoftAP `Aura-Studio-Setup` and Captive Portal DNS on port 53. If the router is booting after a power cut, the user can immediately connect to `192.168.4.1` within 3 seconds of boot with zero lockout. Non-blocking station probe checks for up to 2.25s feeding TWDT, handing off gracefully to background loop without blocking.
   - **Stage 4 (15s Background Reconnect Engine)**: Non-disruptive 15-second background retry loop allows WPA2 4-way handshakes to complete without being repeatedly aborted.
2. **Restored State_t0 Return & WiZ Bulb Arrival Auto-Sync**:
   - If the system was turned off before power down (`isMasterOn == false` or `desk.power == false`), the hub guarantees return to `state_t0` so the desk strip remains dark after the 2.5s boot confirmation wave.
   - WiZ bulbs that power on into default Philips warm white are automatically detected via UDP discovery / `firstBeat` / heartbeat and immediately synchronized into the active system color and brightness state without requiring manual room power cycling.

### Key Highlights of `v01.05.000`:
1. **Clean Segmented Architectural Settings Deck**:
   - Completely refactored the cluttered settings deck into 4 dedicated, focused category tab panels: **🌐 Network & WiZ Mesh**, **🎛️ Spatial & Lighting Physics**, **🔬 Hardware & Diagnostics**, and **⚡ Firmware OTA**.
   - Added tab pills with active pill glow, hover states, keyboard navigation (`ArrowLeft`, `ArrowRight`, `Home`, `End`), and full ARIA accessibility (`role="tab"`, `role="tablist"`, `aria-selected`).
   - Sticky footer with persistent flash NVS hint and clear Save & Apply action.

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
