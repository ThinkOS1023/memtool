#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${ANDROID_NDK:-}" ]]; then
  ANDROID_NDK="$HOME/android-ndk-r29"
fi

if [[ ! -d "${ANDROID_NDK}/build/cmake" ]]; then
  echo "ANDROID_NDK is not set or invalid."
  echo "Set ANDROID_NDK to your NDK root, e.g.:"
  echo "  export ANDROID_NDK=/path/to/android-ndk"
  exit 1
fi

ABI="${ANDROID_ABI:-arm64-v8a}"
PLATFORM="${ANDROID_PLATFORM:-21}"
BUILD_DIR="${BUILD_DIR:-build-android}"

cmake -S . -B "${BUILD_DIR}" \
  -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK}/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="${ABI}" \
  -DANDROID_PLATFORM="${PLATFORM}"

cmake --build "${BUILD_DIR}"
