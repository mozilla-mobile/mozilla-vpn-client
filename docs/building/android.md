
# Android 

## Building the Client (using Dev-Containers) 

1. You need Docker (or compatible) with the ability to run Linux/AMD64 images. 

2. Make sure to run `npm install` 

3. We offer 2 dev-images: 
    - Android-x86_64 - If you are on Intel/AMD and want to run on an Emulator/Chromebook/WSA
    - Android-Arm64 - If you are on an M1+Emulator or want to run this on a real device. 

4. Build a dev container:
    - `npm run android_x86_64:devcontainer:build` 
    - `npm run android_arm64:devcontainer:build`

    This will take a while, grab a coffee! 
5. (Optional): [If you want to use that container with VS-Code learn more](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki/dev-containers)
6. Trigger a Build directly from the Terminal: 
    - `npm run android_x86_64:build`
    - `npm run android_arm64:build`
    
7. You should now have a `debug/` folder containg *.apk

## Cleaning: 
In case you want to purge all build things do `rm .tmp/*` inside the container. 
**Do not remove the .tmp folder** as that is a volume. 

## Installing on a Device / Emulator
1. Install android command-line tools either:
    - `brew install --cask android-platform-tools`
    - `<ANDROID_SDK_PATH>/cmdline-tools/latest/bin/sdkmanager --install "plattform-tools;version`
    - Download [here](https://developer.android.com/tools/releases/platform-tools)

2. Install 
    - Make sure the Device is visible `adb devices` should list your device/emulator
    - `adb install path/to/the.apk`

## Debugging 
1. Install Android Studio
2. Open the mozilla-vpn/android folder as a project. 
3. Wait for the gradle Sync. 
4. Open the app. 
5. In android Studio to go Run->Attach to Process-> Select `org.mozilla.firefox.vpn.debug`
6. Good Luck!

## Archive: Building on your host System. 
> This part of the guide is no longer maintained and might be out of date - Most accurate tip rn is to [Read the dockerfile](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/taskcluster/docker/android-qt6-build/Dockerfile) to see how we setup the build enviroment. 

1. You need to install go >= v1.18. If you don't have it done already, download
it from the [official website](https://golang.org/dl/).

2. Follow the [Getting started](https://doc.qt.io/qt-6/android-getting-started.html) page.

3. Set the `QT_HOST_PATH` environment variable to point to the location of the `androiddeployqt` tool  -- minus the `/bin` suffix i.e. if `$(which androiddeployqt)` is `$HOME/Qt/6.2.4/gcc_64/bin/androiddeployqt`, `QT_HOST_PATH` is `$HOME/Qt/6.2.4/gcc_64/`.

4. Set the `ANDROID_SDK_ROOT` and `ANDROID_NDK_ROOT` environment variables,
to point to the Android SDK and NDK installation directories. Required NDK versions: 23.1.7779620 and 21.0.6113669.

5. Add the Android NDK llvm prebuilt tools to your `PATH`. These are located under the Android NDK installation
directory on `${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/*/bin`.

6. Install the Rust Android targets `rustup target add x86_64-linux-android i686-linux-android armv7-linux-androideabi aarch64-linux-android`.

7. Build the apk
```bash
./scripts/android/cmake.sh -d </path/to/Qt6/> -A <architecture> <debug|release>
```
Add the Adjust SDK token with `-a | --adjust <adjust_token>`.

Valid architecture values: `x86`, `x86_64`, `armeabi-v7a` `arm64-v8a`, by default it will use all.

8. The apk will be located in
`.tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk`

9. Install with adb on device/emulator
```bash
adb install .tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk
```