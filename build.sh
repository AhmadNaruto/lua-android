#!/bin/bash
set -e

# Support ANDROID_NDK_HOME / ANDROID_NDK_ROOT or fallback to default path
NDK="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-$HOME/Android/Sdk/ndk/25.1.8937393}}"

if [ ! -d "$NDK" ]; then
    echo "Error: Android NDK not found at '$NDK'."
    echo "Please set ANDROID_NDK_HOME or ANDROID_NDK_ROOT to your NDK installation directory."
    exit 1
fi

ROOT_DIR="$(pwd)"
ABIS=("armeabi-v7a" "arm64-v8a" "x86" "x86_64")

mkdir -p "$ROOT_DIR/dist/jniLibs"
mkdir -p "$ROOT_DIR/dist/include"
mkdir -p "$ROOT_DIR/dist/kotlin"
cp -r "$ROOT_DIR/include/"* "$ROOT_DIR/dist/include/"
cp -r "$ROOT_DIR/kotlin/"* "$ROOT_DIR/dist/kotlin/"

for ABI in "${ABIS[@]}"; do
    echo "=========================================="
    echo "Configuring and building for $ABI..."
    echo "=========================================="
    BUILD_DIR="$ROOT_DIR/build/lib/$ABI"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    cmake -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
          -DANDROID_ABI="$ABI" \
          -DANDROID_PLATFORM=android-21 \
          "$ROOT_DIR"

    echo "CMake configuration successful for $ABI."
    cmake --build . -- -j$(nproc 2>/dev/null || echo 4)

    echo "Collecting binary to dist/jniLibs/$ABI/..."
    mkdir -p "$ROOT_DIR/dist/jniLibs/$ABI"
    cp "$BUILD_DIR/liblua.so" "$ROOT_DIR/dist/jniLibs/$ABI/"

    cd "$ROOT_DIR"
done

echo "=========================================="
echo "Successfully built for ABIs: ${ABIS[*]}"
echo "Outputs collected in dist/:"
echo " - dist/jniLibs/<abi>/liblua.so"
echo " - dist/include/"
echo "=========================================="
