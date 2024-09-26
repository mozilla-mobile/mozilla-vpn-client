# Build for iOS

Building for iOS can only be done on a Mac.

## Xcode

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

## Activate conda

```bash 
$ conda env create -f env.yml -n vpn
$ conda activate vpn
```

See [here](./index.md#conda) for conda environment instructions.

Install extra conda packages

```bash 
$ conda activate vpn
$ ./scripts/macos/conda_install_extras.sh
```


## Get Qt

```bash 
$ conda activate vpn
$ ./scripts/macos/conda_setup_qt.sh
```


# Build with Xcode

## Configure Xcode build environment

Complete the steps in [macOS.md#configure-xcode-build-environment](./macos.md#configure-xcode-build-environment).

## Build

Configure, using the qt-cmake

    qt-cmake -S . -B build-ios


This will generate an Xcode project file at `build-ios/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

    open build-ios/Mozilla\ VPN.xcodeproj

Once Xcode has opened the project, select the `mozillavpn` target and start the build.

Tips:
* If you can't see a simulator target in the Xcode interface, look at Product -> Destination -> Destination Architectures -> Show Both
* Due to lack of low level networking support, it is not possible to turn on the VPN from the iOS simulator in Xcode.
