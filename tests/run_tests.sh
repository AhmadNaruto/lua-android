#!/bin/bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "=========================================="
echo "Compiling Standalone Lua Test Runner..."
echo "=========================================="

C_SOURCES=$(find src -name "*.c" ! -name "lua.c" ! -name "luac.c" ! -name "lua_jni.c")

clang -Iinclude -Isrc -DLUA_USE_LINUX \
    tests/test_runner.c \
    $C_SOURCES \
    -o tests/test_runner -lm

echo "✓ Test runner compiled successfully."
echo "=========================================="
echo "Executing Functional Tests..."
echo "=========================================="

./tests/test_runner tests/test_suite.lua

echo "=========================================="
echo "All functional tests completed successfully!"
echo "=========================================="
