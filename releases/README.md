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
| **`firmware_01.02.004.bin`** | `v01.02.004` | 2026-11-16 | 1,461,216 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `a6faa97283cf36768d946bdbbc8b23d350c4bd58be6994c0a9d32bb37d88176e` |
| **`firmware_01.02.003.bin`** | `v01.02.003` | 2026-11-16 | 1,459,952 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `40900bcffdfb0d295fa89e218ecc70a9dd5f7f2d74dd857044ac44a53c7aaabb` |
| **`firmware_01.02.002.bin`** | `v01.02.002` | 2026-11-15 | 1,459,056 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `a830f986dcbda748b26e933054614e499b13c2fac8e07734cfa89d0d2da454f9` |
| **`firmware_01.02.001.bin`** | `v01.02.001` | 2026-11-15 | 1,459,072 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `917d3b9c7330c6edc60b50da204c36db01b362eed0e701652c4ae38c861a8275` |
| **`firmware_01.02.000.bin`** | `v01.02.000` | 2026-11-15 | 1,458,336 B (1.39 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `ddf4c4081beed3d9c7e1465205afc7033d2ca33842cf1c019b6d65d4edf68630` |
| **`firmware_01.01.000.bin`** | `v01.01.000` | 2026-11-15 | 1,451,872 B (1.38 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `ff4c315a67b4a7e8a500f6c949fc04dd798a8fd41bab5709f6b888b0c5dc1c02` |
| **`firmware_01.00.000.bin`** | `v01.00.000` | 2026-11-14 | 1,383,995 B (1.32 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `09f5ad864af6807150249f03288c7cd328672cd826af9e3cb405251a488734cf` |

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
