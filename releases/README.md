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
| **`firmware_01.01.000.bin`** | `v01.01.000` | 2026-11-15 | 1,451,872 B (1.38 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `ff4c315a67b4a7e8a500f6c949fc04dd798a8fd41bab5709f6b888b0c5dc1c02` |
| **`firmware_01.00.000.bin`** | `v01.00.000` | 2026-11-14 | 1,383,995 B (1.32 MB) | Dual OTA (`ota_0` / `ota_1`, 1.44 MB) | `09f5ad864af6807150249f03288c7cd328672cd826af9e3cb405251a488734cf` |

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
