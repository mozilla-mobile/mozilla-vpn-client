# Build for iOS

Building for iOS can only be done on a Mac.

## Xcode

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

## Activate conda

If the macOS conda environment has not already been installed, install it:
```bash 
$ conda env create -f env-apple.yml -n vpn
```

Activate the environment:
```bash 
$ conda activate vpn
```

See [here](./index.md#conda) for conda environment instructions.

## Setup Qt 6.10.1

Run the conda setup script.
Confirm the `qt-cmake` being used is the one installed by the script.

```bash
$ ./scripts/macos/conda_setup_qt.sh
$ which qt-cmake # should be ~/miniconda3/envs/vpn/bin/qt-cmake
```

## Create symbolic links for iOS SDKs
**(Only for macOS 26 and higher.)**

_Xcode 26 has changed where iOS SDKs are stored. Symbolic links must be created so `qt-cmake` can find the SDK.
(If a future version of `qt-cmake` changes how it looks for iOS SDKs, this section can be removed.)

```bash
$ cd /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/
$ ln -s iPhoneOS.sdk iPhoneOS26.0.sdk # (for iOS 26.0 - change the version number for other SDK versions)
```

*Important: The specific iOS SDK versions must also be downloaded via Xcode. Creating a symbolic link will not download the SDK automatically.*


## Build

```bash
qt-cmake -S . -B build-ios -GXcode -DQT_HOST_PATH_CMAKE_DIR=~/Qt/6.10.1/macos/lib/cmake -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS26.0.sdk
```

(Older qt-cmake versions used the `-DQT_HOST_PATH=~/Qt/6.6.3/macos` flag rather than `QT_HOST_PATH_CMAKE_DIR` shown here. If having issues, try the older format.)

If you get the error `No CMAKE_Swift_COMPILER could be found.`, include the flag with the direct path the Swift compiler: 
```bash
qt-cmake -S . -B build-ios -GXcode -DQT_HOST_PATH_CMAKE_DIR=~/Qt/6.10.1/macos/lib/cmake -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS26.0.sdk -DCMAKE_Swift_COMPILER=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/swiftc
```

This will generate an Xcode project file at `build-ios/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

    open build-ios/Mozilla\ VPN.xcodeproj

Once Xcode has opened the project, select the `mozillavpn` target and start the build.

Tips:
* If you can't see a simulator target in the Xcode interface, look at Product -> Destination -> Destination Architectures -> Show Both
* Due to lack of low level networking support, it is not possible to turn on the VPN from the iOS simulator in Xcode.
