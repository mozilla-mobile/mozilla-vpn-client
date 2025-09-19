# Build for iOS

Building for iOS can only be done on a Mac.

## Xcode

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

## Activate conda

```bash 
$ conda env create -f env-apple.yml -n vpn
$ conda activate vpn
```

See [here](./index.md#conda) for conda environment instructions.

## Get Qt

```bash 
$ conda activate vpn
$ ./scripts/macos/conda_setup_qt.sh
```


# Build with Xcode

## Configure Xcode build environment

Complete the steps in [macOS.md#configure-xcode-build-environment](./macos.md#configure-xcode-build-environment).

## Create symbolic links for iOS SDKs
**(Only for macOS 26 and higher.)**

_Xcode 26 has changed where iOS SDKs are stored. Symbolic links must be created so `qt-cmake` can find the SDK.
(If a future version of `qt-cmake` changes how it looks for iOS SDKs, this section can be removed.)

```bash
$ cd /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/
$ ln -s iPhoneOS.sdk iPhoneOS18.1.sdk # (for iOS 18.1 - change the version number for other SDK versions)
```

*Important: The specific iOS SDK versions must also be downloaded via Xcode. Creating a symbolic link will not download the SDK automatically.*


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
