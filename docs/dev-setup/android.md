# Pre-requisites

## Setting up a new Conda Env
See [here](./index.md#conda) for general conda environment instructions.

We are going to update the Conda-Env to build for android, as 
this can break building for other plattforms, it's advised to do those steps 
in a seperate enviroment. 

```bash 
$ conda env create -f env.yml -n vpn-android
$ conda activate vpn-android
```

### Optional: Choosing your Android Arch: 
By default this enviroment will be setup to build android-arm64-v8a - 
For real devices or Emulators on M1/M2 macs. 
If you need any other architecture you need to set `ANDROID_ARCH` to the desired arch before continuing.
Valid architecture values: `x86`, `x86_64`, `armeabi-v7a` `arm64-v8a`.
```
$ conda env config vars set ANDROID_ARCH=x86
$ conda deactivate
$ conda activate vpn-android
```
### Optional: Choosing your QT Version: 
By default this enviroment will be setup to use the QT-Version noted in env.yml
If you need any other architecture you need to set `QT_VERSION` before continuing.
```
$ conda env config vars set QT_VERSION=1.2.3
$ conda deactivate
$ conda activate vpn-android
```

### Setup Android-SDK/NDK and QT. 
Now install the Android-SDK dependencies and QT via:
```bash 
# Setup the Android SDK and NDK for the current Conda env.
$ ./scripts/android/conda_setup_sdk.sh
# Setup the Conda env to fetch and use QT
$ ./scripts/android/conda_setup_qt.sh
$ conda deactivate
$ conda activate vpn-android
```


## Build
```
$ conda activate vpn-android
$ ./scripts/android/cmake.sh -d 
```

> For relase builds: Add the Adjust SDK token with `-a | --adjust <adjust_token>`.


# Run

The new apk will be located in

    .tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk
Make sure to Choose the apk-architecture of your env, all others apk's will not work. 

Install with adb on device/[emulator](https://developer.android.com/studio/run/emulator#avd)


    adb install .tmp/src/android-build/build/outputs/apk/debug/<ANDROID_BUILD_ARCHITECTURE>.apk

# Signing (optional)

If you need to work with subscriptions or other play store functionality in the emulator, you will need to sign the apk.

Make sure have an environment variable `AUTOGRAPH_TOKEN` set (ask a fellow dev).

Then run
```
    ./scripts/android/sign.sh .tmp/src/android-build/build/outputs/apk/release
```