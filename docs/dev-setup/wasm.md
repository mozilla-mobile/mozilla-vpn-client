# Pre-requisites

WASM instructions for linux are below. Building for wasm is a little tricky, you may be able to get by using the taskcluster artefacts that run on each PR. Each PR runs a build-wasm/opt job. From github actions view, you can "View task in Taskcluster", then under Artifacts, click on `public/build/index.html`.

If that doesn't work for you, first follow all pre-requisites listed in [linux.md](./linux.md#pre-requisites).

## Other dependencies

Ninja build tool

    sudo apt install ninja-build

Install [emscripten](https://emscripten.org/)

Read the following pages to know more about wasm and Qt:
- https://emscripten.org/docs/getting_started/index.html
- https://doc.qt.io/qt-6/wasm.html

# Build

Make the build directory

    mkdir build

Configure using qt-cmake which is in your installer directory

    <path to installed Qt>/6.2.4/wasm_32/bin/qt-cmake -S . -B build

Compile

    cmake --build build -j$(nproc)
