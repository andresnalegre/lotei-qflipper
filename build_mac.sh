#!/bin/bash

# shellcheck disable=SC2207

set -exuo pipefail;

PROJECT="qFlipper";
BUILD_DIRECTORY="build_mac";

if [ -d ".git" ]; then
    git submodule update --init;
fi

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "This script needs to be run under MacOS";
    exit 1;
fi

if [[ "$(uname -m)" == "arm64" ]]; then
    eval "$(/opt/homebrew/bin/brew shellenv)";
else
    eval "$(/usr/local/Homebrew/bin/brew shellenv)";
fi

# Build for THIS Mac's architecture only. The upstream script forced a universal
# (x86_64 + arm64) build, but on Apple Silicon the project-compiled deps (nanopb
# -> flipperproto0) have no x86_64 slice, so the linker dies with
# "symbol(s) not found for architecture x86_64". Native single-arch just works.
# Override with e.g.  BUILD_ARCHS="arm64" ./build_mac.sh  if you really
# want a universal binary (and have universal deps).
BUILD_ARCHS="${BUILD_ARCHS:-$(uname -m)}";

if ! brew --version; then
    echo "Brew isn't installed!";
    exit 1;
fi

if ! brew --prefix libusb_universal; then
    echo "Please install libusb_universal first!";
    printf "\tbrew install flipperdevices/homebrew-flipper/libusb_universal\n";
    exit 1;
fi

if ! brew --prefix qt_universal; then
    echo "Please install qt_universal first!";
    printf "\tbrew install flipperdevices/homebrew-flipper/qt_universal\n";
    exit 1;
fi

rm -rf "$BUILD_DIRECTORY";
mkdir "$BUILD_DIRECTORY";

cd "$BUILD_DIRECTORY";

qmake \
    -spec macx-clang \
    CONFIG+="release qtquickcompiler" \
    -o Makefile \
    ../$PROJECT.pro \
    QMAKE_APPLE_DEVICE_ARCHS="$BUILD_ARCHS";

make qmake_all;
# NOTE: output is intentionally NOT sent to /dev/null so compile errors are
# visible. (The upstream script hid it, which turns any build failure into a
# silent "the .app doesn't exist".)
make "-j$(sysctl -n hw.ncpu)";
make install;

# bundle libusb
mkdir -p "$PROJECT.app/Contents/Frameworks";
cp "$(brew --prefix libusb_universal)/lib/libusb-1.0.0.dylib" "$PROJECT.app/Contents/Frameworks";

relink_framework()
{
    local FILE;
    local LIB;
    local REL_PATH;
    FILE="$1";
    LIB="$2";
    REL_PATH="$3";
    PATHS=( $(otool -L "$FILE" | grep "$LIB" | awk '{print $1}' ) );
    for CUR in "${PATHS[@]}"; do
        install_name_tool -change "$CUR" "$REL_PATH" "$FILE";
    done
}

relink_framework \
    "$PROJECT.app/Contents/Frameworks/libusb-1.0.0.dylib" \
    "libusb-1.0.0.dylib" \
    "@loader_path/libusb-1.0.0.dylib";
relink_framework \
    "$PROJECT.app/Contents/MacOS/qFlipper" \
    "libusb-1.0.0.dylib" \
    "@loader_path/../Frameworks/libusb-1.0.0.dylib";
relink_framework \
    "$PROJECT.app/Contents/MacOS/qFlipper-cli" \
    "libusb-1.0.0.dylib" \
    "@loader_path/../Frameworks/libusb-1.0.0.dylib";

# Sign
if [ -n "${MAC_OS_SIGNING_KEY_ID:-""}" ]; then
    security default-keychain -s "$MAC_OS_KEYCHAIN_NAME";
    security unlock-keychain -p "$MAC_OS_KEYCHAIN_PASSWORD" "$MAC_OS_KEYCHAIN_NAME";
    xattr -cr "$PROJECT.app";
    codesign --force --options=runtime -s "$MAC_OS_SIGNING_KEY_ID" --deep -v "$PROJECT.app";
    /usr/bin/ditto -c -k --keepParent "$PROJECT.app" "$PROJECT.zip";
    xcrun altool \
        --notarize-app \
        --primary-bundle-id "$MAC_OS_SIGNING_BUNDLE_ID" \
        --username "$MAC_OS_SIGNING_USERNAME" \
        --password "$MAC_OS_SIGNING_PASSWORD" \
        --asc-provider "$MAC_OS_SIGNING_ASC_PROVIDER" \
        --file "$PROJECT.zip";
fi

# build DMG -- optional. Only attempted if dmgbuild is installed; a local dev
# build doesn't need a DMG, and its absence must NOT fail the build (the .app is
# already done above). Install it with `pip3 install dmgbuild` if you want one.
if command -v dmgbuild >/dev/null 2>&1; then
    dmgbuild \
        -s "../installer-assets/macos/dmgbuild-config.py" \
        -D "app=$PROJECT.app" \
        "$PROJECT-$(git describe --tags --abbrev=0 2>/dev/null || echo dev)" \
        "$PROJECT.dmg";
    echo "Built $BUILD_DIRECTORY/$PROJECT.dmg";
else
    echo "dmgbuild not found -- skipping DMG. Your app is ready:";
fi
echo "  $BUILD_DIRECTORY/$PROJECT.app   (run:  open $BUILD_DIRECTORY/$PROJECT.app )";