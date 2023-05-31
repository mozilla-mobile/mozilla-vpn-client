# Pre-requisites

Building for iOS can only be done on a Mac.

## Xcode

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

## Activate conda

    conda activate vpn

See [here](./index.md#conda) for conda environment instructions.

Install extra conda packages

    ./scripts/macos/conda_install_extras.sh

## Get Qt

Get a static build of Qt made built in our CI.

https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-ios.latest/artifacts/public%2Fbuild%2Fqt6_ios.zip

Unzip the folder and remember the location for the configure step.

# Build with Xcode

## Configure Xcode build environment

Complete the steps in [macOS.md#configure-xcode-build-environment](./macos.md#configure-xcode-build-environment).

## Build

Make the build directory

    mkdir build-ios

Configure, using the cmake in the downloaded Qt folder, and setting QT_HOST_PATH to point
to macOS.

    <Qt unzipped path>/6.2.4/ios/bin/qt-cmake -S . -B build-ios -GXcode -DQT_HOST_PATH=<Qt unzipped path>/6.2.4/macos


This will generate an Xcode project file at `build-ios/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

    open build-ios/Mozilla\ VPN.xcodeproj

Once Xcode has opened the project, select the `mozillavpn` target and start the build.

Tips:
* If you can't see a simulator target in the Xcode interface, look at Product -> Destination -> Destination Architectures -> Show Both
* Due to lack of low level networking support, it is not possible to turn on the VPN from the iOS simulator in Xcode.
