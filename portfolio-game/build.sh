#!/bin/bash
# One-command build for Zielony Pływak
# Requires: Docker OR Emscripten SDK
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

if command -v docker &> /dev/null; then
    echo "🐳 Building with Docker..."
    docker build -t zielony-plywak .
    CONTAINER_ID=$(docker create zielony-plywak)
    rm -rf build
    docker cp "$CONTAINER_ID:/src/build" ./build
    docker rm "$CONTAINER_ID" > /dev/null
    echo "✅ Build complete! Output in build/"
    echo "   build/index.html"
    echo "   build/index.js"  
    echo "   build/index.wasm"
    echo ""
    echo "To test: cd build && python3 -m http.server 8080"
elif command -v emcc &> /dev/null; then
    echo "🔧 Building with local Emscripten..."
    make
    echo "✅ Build complete! Output in build/"
else
    echo "❌ Need Docker or Emscripten SDK."
    echo ""
    echo "Easiest: Install Docker, then run ./build.sh"
    exit 1
fi
