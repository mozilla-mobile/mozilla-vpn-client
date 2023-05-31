# Pre-requisites

WASM instructions for Linux are below. Building for wasm is a little tricky, you may be able to get by using the taskcluster artifacts that run on each PR. Each PR runs a build-wasm/opt job. From GitHub actions view, you can "View task in Taskcluster", then under Artifacts, click on `public/build/index.html`.

If that doesn't work for you, first follow all pre-requisites listed in [Linux.md](./linux.md#pre-requisites).

## Other dependencies

Ninja build tool

    sudo apt install ninja-build

Install [emscripten](https://emscripten.org/)

Read the following pages to know more about wasm and Qt:
- https://emscripten.org/docs/getting_started/index.html
- https://doc.qt.io/qt-6/wasm.html

# Build

Make the build directory

```bash
mkdir build
```

Configure using qt-cmake which is in your installer directory

```bash
<path to installed Qt>/6.2.4/wasm_32/bin/qt-cmake -S . -B build
```

Compile

```bash
cmake --build build -j$(nproc)
```

# Run

Run the WASM build for development with

```bash
npm run wasm:dev
```

> **Note**: This assumes the final WASM build folder to be `/build`.
> If the build folder is different from that, set the `WASM_BUILD_DIRECTORY`
> to the chosen build folder.
