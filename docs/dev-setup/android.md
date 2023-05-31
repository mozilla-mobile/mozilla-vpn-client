# Pre-requisites

## Activate conda

`conda activate vpn`

See [here](./index.md#conda) for conda environment instructions.

## Get Qt

Download the online installer from the official QT website: https://www.qt.io/download-qt-installer

Our usage is covered by the QT open source license.

While you will need to register with an email address, it will be free.

During the install, there will be many components available.
Install all of Qt version 6.2.4 and you'll have everything you need.

Finally, follow the Qt [Android getting started](https://doc.qt.io/qt-6/android-getting-started.html) page.
Deep breath, this is quite a lot of steps.

## Other dependencies

Until conda has an android target, you'll need to install rust with rustup so you can add an android target.

Follow directions at https://www.rust-lang.org/tools/install

Then run

    rustup target add x86_64-linux-android i686-linux-android armv7-linux-androideabi aarch64-linux-android

The android build additionally requires cmake 3.10.2. You can install it using: `./sdkmanager --install "cmake;3.10.2.4988404"`.
Note that `sdkmanager` should have been installed when you followed the Qt Android getting started.

# Build and install

## Configure

Set the `QT_HOST_PATH` environment variable to point to the location of the `androiddeployqt` tool  -- minus the `/bin` suffix i.e. if `$(which androiddeployqt)` is `$HOME/Qt/6.2.4/gcc_64/bin/androiddeployqt`, `QT_HOST_PATH` is `$HOME/Qt/6.2.4/gcc_64/`.

Set the `ANDROID_SDK_ROOT` and `ANDROID_NDK_ROOT` environment variables,
to point to the Android SDK and NDK installation directories. Required NDK versions: 23.1.7779620 and 21.0.6113669.

Add the Android NDK llvm prebuilt tools to your `PATH`. These are located under the Android NDK installation
directory on `${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/*/bin`.

## Build

./scripts/android/cmake.sh -d </path/to/Qt/> -A <architecture> <debug|release>

If needed, add the Adjust SDK token with `-a | --adjust <adjust_token>`.

Valid architecture values: `x86`, `x86_64`, `armeabi-v7a` `arm64-v8a`, by default it will use all.

# Run

The new apk will be located in

    `.tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk`

Install with adb on device/emulator

    adb install .tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk

# Signing (optional)

If you need to work with subscriptions or other play store functionality in the emulator, you will need to sign the apk.

Make sure have an environment variable `AUTOGRAPH_TOKEN` set (ask a fellow dev).

Then run

    ./scripts/android/sign.sh
