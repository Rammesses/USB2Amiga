#!/usr/bin/env bash
# build.sh — configure and build USB2Amiga firmware
# Usage: ./build.sh [clean]
set -e

PICO_SDK_PATH="${PICO_SDK_PATH:-$HOME/pico-sdk}"

if [ ! -d "$PICO_SDK_PATH" ]; then
    echo "ERROR: Pico SDK not found at $PICO_SDK_PATH"
    echo "Set PICO_SDK_PATH or run: git clone --depth 1 --branch 1.5.1 https://github.com/raspberrypi/pico-sdk.git ~/pico-sdk && cd ~/pico-sdk && git submodule update --init --depth 1"
    exit 1
fi

if [ "$1" = "clean" ]; then
    rm -rf build
fi

mkdir -p build
cd build
PICO_SDK_PATH="$PICO_SDK_PATH" cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug --log-level=WARNING
ninja
echo ""
echo "Build complete: build/usb2amiga.uf2"
