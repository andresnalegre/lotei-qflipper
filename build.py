#!/usr/bin/env python3
"""
build.py - one-shot build & run for the LOTEI qFlipper fork (macOS, Apple Silicon).

What it does:
  1. Forces the build to arm64 (avoids the universal x86_64 linker failure).
  2. Clean build:  rm -rf build_mac && ./build_mac.sh
  3. Copies the protobuf plugin into the .app (so RPC works).
  4. Clears the Gatekeeper quarantine and opens the app.

Usage:
    python3 build.py            # full clean build + run
    python3 build.py --fast     # incremental build (no rm -rf), much quicker
    python3 build.py --no-open  # build only, don't launch the app
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

PROJECT = Path(__file__).resolve().parent          # ~/Desktop/lotei-qflipper
BUILD = PROJECT / "build_mac"
APP = BUILD / "qFlipper.app"
PLUGIN = BUILD / "plugins" / "libflipperproto0.dylib"
PLUGINS_DIR = APP / "Contents" / "PlugIns"

FAST = "--fast" in sys.argv
NO_OPEN = "--no-open" in sys.argv


def c(txt, code):
    return f"\033[{code}m{txt}\033[0m"


def step(msg):
    print(c(f"\n▶ {msg}", "95;1"))   # bright magenta


def ok(msg):
    print(c(f"  ✓ {msg}", "92"))     # green


def die(msg):
    print(c(f"  ✗ {msg}", "91;1"))   # bright red
    sys.exit(1)


def run(cmd, **kw):
    """Run a shell command, streaming output. Returns the CompletedProcess."""
    return subprocess.run(cmd, shell=True, cwd=PROJECT, **kw)


def ensure_arm64():
    """Make build_mac.sh build arm64-only (the fix we rely on)."""
    script = PROJECT / "build_mac.sh"
    if not script.exists():
        die("build_mac.sh not found - are you in the project folder?")
    text = script.read_text()
    if "x86_64 arm64" in text:
        step("Patching build_mac.sh to arm64-only")
        script.write_text(text.replace("x86_64 arm64", "arm64"))
        ok("build_mac.sh set to arm64")


def build():
    if FAST:
        step("Incremental build (make -j)")
        if not BUILD.exists():
            die("build_mac/ doesn't exist yet - run a full build first (no --fast)")
        r = run(f"make -C '{BUILD}' -j$(sysctl -n hw.ncpu)")
        if r.returncode != 0:
            die("build failed - scroll up for the first 'error:' line")
    else:
        step("Clean build (rm -rf build_mac && ./build_mac.sh)")
        if BUILD.exists():
            shutil.rmtree(BUILD)
        r = run("./build_mac.sh")
        # build_mac.sh may exit non-zero on the optional DMG step; we check the
        # real artifact below instead of trusting the return code.

    # verify there were no compile errors by checking the executable exists
    exe = APP / "Contents" / "MacOS" / "qFlipper"
    if not exe.exists():
        die("build did not produce qFlipper.app - there was a compile error. "
            f"Run:  make -C '{BUILD}'   to see it.")
    ok("app built")


def bundle_plugin():
    step("Bundling protobuf plugin into the .app")
    if not PLUGIN.exists():
        die(f"plugin missing: {PLUGIN}")
    PLUGINS_DIR.mkdir(parents=True, exist_ok=True)
    shutil.copy2(PLUGIN, PLUGINS_DIR / PLUGIN.name)
    ok(f"copied {PLUGIN.name} -> Contents/PlugIns/")


def launch():
    step("Clearing quarantine and opening")
    # -r recurses; ignore the harmless 'permission denied' on read-only libusb
    run(f"xattr -cr '{APP}' 2>/dev/null")
    if NO_OPEN:
        ok(f"done. Run it with:  open '{APP}'")
    else:
        run(f"open '{APP}'")
        ok("launched")


def main():
    print(c("═══ LOTEI qFlipper build ═══", "95;1"))
    if not FAST:
        ensure_arm64()
    build()
    bundle_plugin()
    launch()
    print(c("\n✔ all done\n", "92;1"))


if __name__ == "__main__":
    main()