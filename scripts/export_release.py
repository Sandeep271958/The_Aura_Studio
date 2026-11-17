#!/usr/bin/env python3
"""
The Aura Studio - Automated Firmware Release & Nomenclature Exporter
Standardizes firmware artifact packaging to: firmware_XX.YY.ZZZ.bin

Features & Pre-Flight Guards:
1. Pre-flight Guard 1: Runs test_suite.js and asserts 100% pass before build.
2. Pre-flight Guard 2: Verifies dashboard/ and firmware/data/ 100% byte-for-byte mirror parity.
3. Compilation: Builds firmware via PlatformIO (pio run -d firmware).
4. Guard 3 (Flash Budget): Asserts binary size <= 1,507,328 B (0x170000) with >= 50KB headroom.
5. Packaging: Copies binary to firmware_{version}.bin and firmware_latest.bin.
6. Guard 4 (Registry Sync): Automatically parses and updates releases/README.md.
"""

import os
import re
import sys
import shutil
import hashlib
import subprocess
import datetime

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
CONFIG_PATH = os.path.join(PROJECT_ROOT, "firmware", "src", "Config.h")
BUILD_BIN = os.path.join(PROJECT_ROOT, "firmware", ".pio", "build", "esp32dev", "firmware.bin")
RELEASES_DIR = os.path.join(PROJECT_ROOT, "releases")
DASHBOARD_DIR = os.path.join(PROJECT_ROOT, "dashboard")
DATA_DIR = os.path.join(PROJECT_ROOT, "firmware", "data")
README_PATH = os.path.join(RELEASES_DIR, "README.md")

# Dual-partition OTA allocation limit from partitions_ota.csv (0x170000)
MAX_FLASH_BUDGET = 1507328  # 1,507,328 bytes (1.4375 MB)
MIN_HEADROOM_BYTES = 50 * 1024  # 51,200 bytes (50 KB safety margin)
VERSION_REGEX = r'^\d{2}\.\d{2}\.\d{3}$'

def get_firmware_version():
    """Extracts FIRMWARE_VERSION string from Config.h with format validation."""
    with open(CONFIG_PATH, "r", encoding="utf-8") as f:
        content = f.read()
    match = re.search(r'#define\s+FIRMWARE_VERSION\s+"([^"]+)"', content)
    if not match:
        raise ValueError("Could not find FIRMWARE_VERSION definition in Config.h")
    version = match.group(1)
    if not re.match(VERSION_REGEX, version):
        raise ValueError(f"Invalid FIRMWARE_VERSION format: '{version}'. Expected XX.YY.ZZZ standard (e.g. 01.01.000)")
    return version

def get_firmware_metadata():
    """Extracts version and build date from Config.h."""
    version = get_firmware_version()
    with open(CONFIG_PATH, "r", encoding="utf-8") as f:
        content = f.read()
    build_match = re.search(r'#define\s+FIRMWARE_BUILD\s+"([^"]+)"', content)
    build_date = build_match.group(1) if build_match else datetime.date.today().isoformat()
    return version, build_date

def sha256sum(filename):
    """Computes SHA-256 hash using 64KB buffered stream."""
    h = hashlib.sha256()
    with open(filename, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            h.update(chunk)
    return h.hexdigest()

def guard_run_test_suite():
    """Pre-flight Guard 1: Run automated test suite and require exit code 0."""
    print("▶ [Guard 1/4] Running automated test suite (node test_suite.js)...")
    try:
        res = subprocess.run(["node", "test_suite.js"], cwd=PROJECT_ROOT)
        if res.returncode != 0:
            print(f"\n❌ Pre-Flight Guard 1 FAILED: node test_suite.js exited with code {res.returncode}.", file=sys.stderr)
            print("   Aborting release packaging to prevent shipping unverified code.\n", file=sys.stderr)
            sys.exit(res.returncode)
        print("✔ [Guard 1/4] Automated test suite passed 100% with zero regressions.\n")
    except FileNotFoundError:
        print("❌ Pre-Flight Guard 1 FAILED: 'node' executable not found in PATH.", file=sys.stderr)
        sys.exit(1)

def guard_verify_asset_parity():
    """Pre-flight Guard 2: Verify dashboard/ and firmware/data/ match 100% byte-for-byte."""
    print("▶ [Guard 2/4] Verifying byte-for-byte parity (dashboard/ vs firmware/data/)...")
    if not os.path.exists(DASHBOARD_DIR):
        print(f"❌ Pre-Flight Guard 2 FAILED: Dashboard directory missing: {DASHBOARD_DIR}", file=sys.stderr)
        sys.exit(1)
    if not os.path.exists(DATA_DIR):
        print(f"❌ Pre-Flight Guard 2 FAILED: Firmware data directory missing: {DATA_DIR}", file=sys.stderr)
        sys.exit(1)

    def get_rel_files(base_dir):
        rel_files = []
        for root, _, files in os.walk(base_dir):
            for f in files:
                rel_files.append(os.path.relpath(os.path.join(root, f), base_dir))
        return sorted(rel_files)

    dash_files = get_rel_files(DASHBOARD_DIR)
    fw_files = get_rel_files(DATA_DIR)

    missing = [f for f in dash_files if f not in fw_files]
    extraneous = [f for f in fw_files if f not in dash_files]

    if missing or extraneous:
        print(f"❌ Pre-Flight Guard 2 FAILED: File symmetry mismatch!", file=sys.stderr)
        if missing:
            print(f"   Missing in firmware/data/: {missing}", file=sys.stderr)
        if extraneous:
            print(f"   Extraneous in firmware/data/: {extraneous}", file=sys.stderr)
        sys.exit(1)

    mismatches = []
    for f in dash_files:
        p_dash = os.path.join(DASHBOARD_DIR, f)
        p_fw = os.path.join(DATA_DIR, f)
        with open(p_dash, "rb") as fd, open(p_fw, "rb") as ff:
            dash_bytes = fd.read()
            fw_bytes = ff.read()
            if len(dash_bytes) != len(fw_bytes) or dash_bytes != fw_bytes:
                mismatches.append(f)

    if mismatches:
        print(f"❌ Pre-Flight Guard 2 FAILED: Byte differences detected in: {mismatches}", file=sys.stderr)
        sys.exit(1)

    print(f"✔ [Guard 2/4] Asset parity verified: {len(dash_files)} assets 100% byte-for-byte identical.\n")

def guard_check_flash_budget(binary_path):
    """Post-compilation Guard 3: Assert compiled binary <= 1,507,328 B with >= 50KB headroom."""
    print("▶ [Guard 3/4] Validating dual-partition OTA flash memory budget...")
    file_size = os.path.getsize(binary_path)
    if file_size > MAX_FLASH_BUDGET:
        overflow = file_size - MAX_FLASH_BUDGET
        print(f"\n❌ Guard 3 FAILED: Binary size {file_size:,} B exceeds partition budget of {MAX_FLASH_BUDGET:,} B (0x170000) by {overflow:,} B!", file=sys.stderr)
        print("   OTA flashing would fail or overwrite adjacent partition. Aborting.\n", file=sys.stderr)
        sys.exit(1)

    headroom = MAX_FLASH_BUDGET - file_size
    headroom_pct = (headroom / MAX_FLASH_BUDGET) * 100
    print(f"✔ [Guard 3/4] Flash budget check passed:")
    print(f"   Binary size:  {file_size:,} B ({file_size / (1024 * 1024):.2f} MB)")
    print(f"   Budget limit: {MAX_FLASH_BUDGET:,} B (0x170000 / 1.44 MB)")
    print(f"   Headroom:     {headroom:,} B ({headroom / 1024:.2f} KB / {headroom_pct:.2f}%)")

    if headroom < MIN_HEADROOM_BYTES:
        print(f"⚠️ Warning: Headroom ({headroom / 1024:.2f} KB) is below recommended 50 KB safety margin!\n", file=sys.stderr)
    else:
        print(f"   Safety margin: Headroom exceeds 50 KB required reserve.\n")

    return file_size

def guard_update_readme_registry(version, out_name, build_date, file_size, checksum):
    """Post-packaging Guard 4: Parse/update releases/README.md markdown registry."""
    print("▶ [Guard 4/4] Updating release registry in releases/README.md...")
    if not os.path.exists(README_PATH):
        print(f"Warning: {README_PATH} not found. Skipping registry update.", file=sys.stderr)
        return False

    with open(README_PATH, "r", encoding="utf-8") as f:
        content = f.read()

    file_size_mb = file_size / (1024 * 1024)
    formatted_size = f"{file_size:,} B ({file_size_mb:.2f} MB)"
    partition_target = "Dual OTA (`ota_0` / `ota_1`, 1.44 MB)"
    new_row = f"| **`{out_name}`** | `v{version}` | {build_date} | {formatted_size} | {partition_target} | `{checksum}` |"

    # Match row for this version if already present
    row_pattern = rf"\|\s*\*\*`[^`]+`\*\*\s*\|\s*`v{re.escape(version)}`\s*\|[^\n]+"

    if re.search(row_pattern, content):
        content = re.sub(row_pattern, new_row, content, count=1)
        action = "Updated existing"
    else:
        table_sep = "| :--- | :--- | :--- | :--- | :--- | :--- |"
        if table_sep in content:
            content = content.replace(table_sep, f"{table_sep}\n{new_row}", 1)
            action = "Appended new"
        else:
            content += f"\n{new_row}\n"
            action = "Appended"

    with open(README_PATH, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"✔ [Guard 4/4] {action} registry entry in releases/README.md:")
    print(f"   {new_row}\n")
    return True

def main():
    version, build_date = get_firmware_metadata()
    print("================================================================================")
    print("✦ The Aura Studio — Automated Firmware Release & Nomenclature Exporter")
    print("================================================================================")
    print(f"Detected Firmware Version: v{version}")
    print(f"Detected Build Date:      {build_date}\n")

    # Pre-Flight Guard 1: Test Suite
    guard_run_test_suite()

    # Pre-Flight Guard 2: Web Asset Parity
    guard_verify_asset_parity()

    # Compilation
    print("▶ Compiling firmware via PlatformIO...")
    pio_bin = "pio" if shutil.which("pio") else "/Library/Frameworks/Python.framework/Versions/3.11/bin/pio"
    res = subprocess.run([pio_bin, "run", "-d", os.path.join(PROJECT_ROOT, "firmware")], check=True)

    if not os.path.exists(BUILD_BIN):
        print(f"Error: Build binary not found at {BUILD_BIN}", file=sys.stderr)
        sys.exit(1)

    # Post-Compilation Guard 3: Partition Flash Budget Check
    file_size = guard_check_flash_budget(BUILD_BIN)

    # Packaging artifacts
    os.makedirs(RELEASES_DIR, exist_ok=True)
    out_name = f"firmware_{version}.bin"
    target_path = os.path.join(RELEASES_DIR, out_name)
    latest_path = os.path.join(RELEASES_DIR, "firmware_latest.bin")

    shutil.copyfile(BUILD_BIN, target_path)
    shutil.copyfile(BUILD_BIN, latest_path)

    file_size_mb = file_size / (1024 * 1024)
    checksum = sha256sum(target_path)

    # Build LittleFS filesystem image
    print("▶ Building LittleFS filesystem image via PlatformIO...")
    subprocess.run([pio_bin, "run", "--target", "buildfs", "-d", os.path.join(PROJECT_ROOT, "firmware")], check=True)
    build_fs = os.path.join(PROJECT_ROOT, "firmware", ".pio", "build", "esp32dev", "littlefs.bin")
    if os.path.exists(build_fs):
        fs_target_path = os.path.join(RELEASES_DIR, f"littlefs_{version}.bin")
        fs_latest_path = os.path.join(RELEASES_DIR, "littlefs_latest.bin")
        shutil.copyfile(build_fs, fs_target_path)
        shutil.copyfile(build_fs, fs_latest_path)
        fs_size = os.path.getsize(fs_target_path)
        fs_checksum = sha256sum(fs_target_path)
        print(f"✔ LittleFS Web Dashboard Packaged: {fs_target_path} ({fs_size:,} bytes)")

    # Post-Packaging Guard 4: Registry Update
    guard_update_readme_registry(version, out_name, build_date, file_size, checksum)

    print("================================================================================")
    print("✅ Firmware & Dashboard Release Packaged Successfully!")
    print("================================================================================")
    print(f"  Firmware:   {target_path} ({file_size:,} bytes)")
    print(f"  SHA-256:    {checksum}")
    if os.path.exists(build_fs):
        print(f"  Filesystem: {os.path.join(RELEASES_DIR, 'littlefs_latest.bin')} ({os.path.getsize(build_fs):,} bytes)")
    print(f"  Format:     firmware_XX.YY.ZZZ.bin & littlefs_latest.bin\n")

if __name__ == "__main__":
    main()
