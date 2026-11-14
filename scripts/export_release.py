#!/usr/bin/env python3
"""
The Aura Studio - Automated Firmware Release & Nomenclature Exporter
Standardizes firmware artifact packaging to: firmware_XX.YY.ZZZ.bin
"""

import os
import re
import sys
import shutil
import hashlib
import subprocess

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
CONFIG_PATH = os.path.join(PROJECT_ROOT, "firmware", "src", "Config.h")
BUILD_BIN = os.path.join(PROJECT_ROOT, "firmware", ".pio", "build", "esp32dev", "firmware.bin")
RELEASES_DIR = os.path.join(PROJECT_ROOT, "releases")

def get_firmware_version():
    with open(CONFIG_PATH, "r", encoding="utf-8") as f:
        content = f.read()
    match = re.search(r'#define\s+FIRMWARE_VERSION\s+"([^"]+)"', content)
    if not match:
        raise ValueError("Could not find FIRMWARE_VERSION definition in Config.h")
    return match.group(1)

def sha256sum(filename):
    h = hashlib.sha256()
    with open(filename, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            h.update(chunk)
    return h.hexdigest()

def main():
    version = get_firmware_version()
    print(f"✦ The Aura Studio Firmware Exporter")
    print(f"Detected Firmware Version: v{version}")
    
    # 1. Compile firmware
    print("Compiling firmware via PlatformIO...")
    res = subprocess.run(["pio", "run", "-d", os.path.join(PROJECT_ROOT, "firmware")], check=True)
    
    if not os.path.exists(BUILD_BIN):
        print(f"Error: Build binary not found at {BUILD_BIN}", file=sys.stderr)
        sys.exit(1)
        
    os.makedirs(RELEASES_DIR, exist_ok=True)
    out_name = f"firmware_{version}.bin"
    target_path = os.path.join(RELEASES_DIR, out_name)
    latest_path = os.path.join(RELEASES_DIR, "firmware_latest.bin")
    
    shutil.copyfile(BUILD_BIN, target_path)
    shutil.copyfile(BUILD_BIN, latest_path)
    
    file_size = os.path.getsize(target_path)
    file_size_mb = file_size / (1024 * 1024)
    checksum = sha256sum(target_path)
    
    print("\n✅ Firmware Release Packaged Successfully!")
    print(f"  Artifact:  {target_path}")
    print(f"  Latest:    {latest_path}")
    print(f"  Size:      {file_size} bytes ({file_size_mb:.2f} MB)")
    print(f"  SHA-256:   {checksum}")
    print(f"  Format:    firmware_XX.YY.ZZZ.bin (Industry Standard)\n")

if __name__ == "__main__":
    main()
