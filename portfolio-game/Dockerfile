FROM emscripten/emsdk:3.1.50

WORKDIR /src
COPY src/main.cpp src/main.cpp
COPY shell.html shell.html

RUN mkdir -p build && \
    em++ src/main.cpp -o build/index.html \
    -s USE_SDL=2 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s TOTAL_MEMORY=67108864 \
    -s NO_EXIT_RUNTIME=1 \
    -O2 \
    --shell-file shell.html \
    -s "EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

# Output: build/index.html, build/index.js, build/index.wasm
