#!/usr/bin/env bash
# ==============================================================================
# The Aura Studio - Automated USB Emergency Recovery Flasher
# Used when the ESP32 is in a bootloop or OTA is inaccessible
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BINARY="$PROJECT_ROOT/releases/firmware_01.07.001.bin"

echo "========================================================================"
echo "   ✦ THE AURA STUDIO — USB RECOVERY FLASHER (v01.07.001)"
echo "========================================================================"
echo ""

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Release binary not found at: $BINARY"
    exit 1
fi

echo "✔ Target Binary: $BINARY"
echo "  File Size:     $(wc -c < "$BINARY" | tr -d ' ') bytes"
echo ""
echo "Instructions:"
echo "1. Connect your ESP32 to your computer via a micro-USB / USB-C data cable."
echo "2. If the ESP32 is rebooting repeatedly:"
echo "   - Press and HOLD the 'BOOT' (or 'IO0') button on the ESP32."
echo "   - While holding 'BOOT', momentarily press the 'EN' (or 'RST') button."
echo "   - Release the 'BOOT' button. (The ESP32 is now in hardware ROM bootloader mode)."
echo ""

read -p "Press [Enter] when the ESP32 is connected to start flashing..."

echo ""
echo "▶ Detecting ESP32 and flashing firmware..."

if command -v pio >/dev/null 2>&1; then
    echo "Using PlatformIO to upload firmware..."
    pio run -d "$PROJECT_ROOT/firmware" -t upload
    exit 0
elif [ -f "/Library/Frameworks/Python.framework/Versions/3.11/bin/pio" ]; then
    echo "Using PlatformIO (Python 3.11) to upload firmware..."
    /Library/Frameworks/Python.framework/Versions/3.11/bin/pio run -d "$PROJECT_ROOT/firmware" -t upload
    exit 0
fi

if command -v esptool.py >/dev/null 2>&1; then
    echo "Using esptool.py to upload binary..."
    esptool.py write_flash 0x10000 "$BINARY"
    exit 0
elif python3 -m esptool version >/dev/null 2>&1; then
    echo "Using python3 -m esptool to upload binary..."
    python3 -m esptool write_flash 0x10000 "$BINARY"
    exit 0
fi

echo "❌ Neither 'pio' nor 'esptool' found in PATH."
echo "Please run:"
echo "  /Library/Frameworks/Python.framework/Versions/3.11/bin/pio run -d firmware -t upload"
exit 1
