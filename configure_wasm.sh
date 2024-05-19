#/bin/sh
export EMSCRIPTEN=/usr/lib/emscripten
cmake -S ./ -B ./build-wasm/ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_TOOLCHAIN_FILE="${EMSCRIPTEN}/cmake/Modules/Platform/Emscripten.cmake" -DCMAKE_BUILD_TYPE=Debug -DYGL_NO_ASSIMP=0 -DYGL_STATIC=1 -DASAN_DETECT_LEAKS=0
