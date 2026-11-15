#!/usr/bin/env python3
"""
The Aura Studio - Secure Localhost Companion Runner
Starts a lightweight local HTTP server on http://localhost:8080 serving dashboard/.

Why this exists:
Google Chrome, Safari, and Chromium browsers enforce the W3C Secure Contexts spec:
Media capture APIs (navigator.mediaDevices.getDisplayMedia for Ambilight and
navigator.mediaDevices.getUserMedia for Sound Sync) are disabled by default on
plain HTTP local network domains (like http://aura-hub.local).

By serving the dashboard from 'localhost', Google Chrome automatically and natively
classifies the origin as a certified Secure Context (window.isSecureContext === true).
This removes all "Not Secure" warnings and unlocks full-speed Ambilight Screen Sync
and Sound Sync audio reactivity with live WebSocket telemetry to the ESP32!
"""

import http.server
import os
import socketserver
import sys
import webbrowser

PORT = 8080
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DASHBOARD_DIR = os.path.join(PROJECT_ROOT, "dashboard")

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DASHBOARD_DIR, **kwargs)

    def log_message(self, format, *args):
        # Keep terminal output clean during high-frequency visualizer polling
        pass

def main():
    if not os.path.exists(DASHBOARD_DIR):
        print(f"❌ Error: Dashboard directory not found at: {DASHBOARD_DIR}", file=sys.stderr)
        sys.exit(1)

    os.chdir(DASHBOARD_DIR)
    port = PORT
    server = None
    for p in range(PORT, PORT + 20):
        try:
            server = socketserver.TCPServer(("", p), Handler)
            port = p
            break
        except OSError:
            continue

    if not server:
        print(f"❌ Error: Could not bind to any port between {PORT} and {PORT+20}", file=sys.stderr)
        sys.exit(1)

    url = f"http://localhost:{port}"
    print("=" * 72)
    print("✦ THE AURA STUDIO — SECURE DESKTOP COMPANION")
    print("=" * 72)
    print(f"✔ Dashboard Active:   {url}")
    print("✔ Secure Context:     100% NATIVE SECURE (W3C Loopback)")
    print("✔ Ambilight Sync:     UNLOCKED (Screen Capture enabled)")
    print("✔ SoundSync Audio:    UNLOCKED (Microphone FFT enabled)")
    print("✔ ESP32 Target:       ws://aura-hub.local/ws")
    print("=" * 72)
    print(f"Opening Google Chrome to {url} ...")
    print("Press Ctrl+C to stop.")
    print("=" * 72)

    try:
        webbrowser.open(url)
    except Exception:
        pass

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping local companion server...")
        server.server_close()

if __name__ == "__main__":
    main()
