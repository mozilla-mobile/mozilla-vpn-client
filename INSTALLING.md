---


---

# Installing 

Download pre-built binaries for a release

There are pre-built binaries of the last released version of Mozilla VPN for Windows, Mac, Linux, Android iOS and Wasm. 

## Table Of Contents

- [Linux](#linux)
    - [Flatpak](#flatpak)
    - [Debian & Ubuntu (.deb)](#debian--ubuntu-deb)
    - [Fedora (.rpm)](#fedora-rpm)
    - [From Source](#from-source)
- [Android](#android)
    - [Google Play](#google-play)
    - [Non Google Play devices](#non-google-play-devices)
    - [From Source](#from-source-1)
- [Windows](#windows)
    - [Windows 10&11 x64](#windows-1011-x64)
    - [Windows 11 aarch64](#windows-11-aarch64)
    - [From Source](#from-source-2)
- [Macos](#macos)
    - [From Source](#from-source-3)
- [iOS](#ios)
    - [From Source](#from-source-4)
- [WASM](#wasm)
    - [From Taskcluster artifacts](#from-taskcluster-artifacts)
    - [From Source](#from-source-5)




## Linux

### Flatpak

The Mozilla VPN client supports building and distribution via flatpak, and is
available on Flathub at [org.mozilla.vpn](https://flathub.org/apps/org.mozilla.vpn).

#### Building the Flatpak
The flatpak manifests for the latest stable release can be found at
[flathub/org.mozilla.vpn](https://github.com/flathub/org.mozilla.vpn), and this
project also contains a manifest for local development in the `linux/flatpak`
directory.

To build the flatpak package, start by installing the `org.flatpak.Builder` app:

```
flatpak install -y flathub org.flatpak.Builder
```

Add the Flathub repo user-wide:

```
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
```

Then build the project manifest and install it:

```
flatpak run org.flatpak.Builder --force-clean --sandbox --user --install --install-deps-from=flathub --ccache build-flatpak linux/flatpak/org.mozilla.vpn.yml
```

Once installed, the Mozilla VPN client can be run using the `org.mozilla.vpn`
app identifier:

```
   flatpak run org.mozilla.vpn
```

### Debian & Ubuntu (.deb)

#### Mozilla APT repository (recommended)

Supported on Ubuntu 22.04+, Linux Mint 20+, and Debian 11+.

```bash
sudo install -d -m 0755 /etc/apt/keyrings
wget -q https://packages.mozilla.org/apt/repo-signing-key.gpg -O- | sudo tee /etc/apt/keyrings/packages.mozilla.org.asc > /dev/null
```

Verify the key fingerprint matches `35BAA0B33E9EB396F59CA838C0BA5CE6DC6315A3`:

```bash
gpg -n -q --import --import-options import-show /etc/apt/keyrings/packages.mozilla.org.asc | awk '/pub/{getline; gsub(/^ +| +$/,""); if($0 == "35BAA0B33E9EB396F59CA838C0BA5CE6DC6315A3") print "\nThe key fingerprint matches ("$0").\n"; else print "\nVerification failed: the fingerprint ("$0") does not match the expected one.\n"}'
```

Add the repository and install:

```bash
echo "deb [signed-by=/etc/apt/keyrings/packages.mozilla.org.asc] https://packages.mozilla.org/apt mozilla main" | sudo tee -a /etc/apt/sources.list.d/mozilla.list > /dev/null
sudo apt-get update && sudo apt-get install mozillavpn
```

#### Manual .deb download

Past releases are available at [archive.mozilla.org](https://archive.mozilla.org/pub/vpn/releases/). Download the `.deb` for your release and install with:

```bash
sudo dpkg -i mozillavpn-<version>.deb
```

### Fedora (.rpm)

RPM packages are available at [archive.mozilla.org](https://archive.mozilla.org/pub/vpn/releases/). Download the `.rpm` for your release and install with:

```bash
sudo dnf install ./mozillavpn-<version>.rpm
```

### From Source

Clone the repository and install build dependencies.

```bash
# For Debian-ish
sudo apt-get install devscripts equivs
mk-build-deps --install --remove linux/debian/control
# For Fedora-ish
sudo dnf builddep --define "_version 0" linux/mozillavpn.spec
```

Then install the Python build dependencies:

```bash
pip install -r requirements.txt
```

Then configure and build:

```bash
cmake -S . -B build -GNinja
cmake --build build
```

The built binary is at `build/src/mozillavpn`. Note that `wireguard-tools` is also required at runtime.
You can run the app with `mozillavpn`
Alternatively, you can use two terminals to run the daemon manually 
`sudo mozillavpn linuxdaemon` and separately `mozillavpn `


## Android

### Google Play

We are in the [Google Play Store](https://play.google.com/store/apps/details?id=org.mozilla.firefox.vpn) older versions can be fetched from [archive.mozilla.org](https://archive.mozilla.org/pub/vpn/releases/). 

### Non Google Play devices

For devices that do not support Google-Play Services, a non gps build can be fetched from [archive.mozilla.org](https://archive.mozilla.org/pub/vpn/releases/) - check for the android/foss-* apks.

### From Source 

Please install [miniconda](https://docs.conda.io/en/latest/miniconda.html).

```bash 
$ conda env create -f env-android.yml
$ conda activate vpn-android
# Optional: If another aarch then arm64-v8a is needed do:
$ conda env config vars set ANDROID_ARCH=x86
# Valid architecture values: `x86`, `x86_64`, `armeabi-v7a` `arm64-v8a`.
# Optional: If another QT Version is needed do: 
$ conda env config vars set QT_VERSION=1.2.3
# Re-Activate the ENV to apply changes. 
$ conda deactivate
$ conda activate vpn-android

# Setup the Android SDK and NDK for the current Conda env.
$ ./scripts/android/conda_setup_sdk.sh
# Setup the Conda env to fetch and use QT
$ ./scripts/android/conda_setup_qt.sh
$ conda deactivate
$ conda activate vpn-android
```

Trigger a build with:
```
$ ./scripts/android/cmake.sh -d 
```

## Windows 

### Windows 10&11 x64
Get the installer of the latest release [here](https://vpn.mozilla.org/r/vpn/download/windows), you can find past versions on [archive.mozilla.org](https://archive.mozilla.org/pub/vpn/releases/). 

### Windows 11 aarch64

[!CAUTION]
This is not Stable yet! Do at your own risk. 

We don't have any stable arm64 builds yet, you can however help us test by grabbing the latest build of [main directly from our CI](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozilla-vpn-client.branch.main.latest.build.msi-aarch64/artifacts/public%2Fbuild%2FMozillaVPN-aarch64.msi)


### From Source 

Have [miniconda installed](https://repo.anaconda.com/miniconda/Miniconda3-py310_23.1.0-1-Windows-x86_64.exe).


[!INFO]
You may need to enable [powershell scripts](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.security/set-executionpolicy?view=powershell-7.3). You can call ```Set-ExecutionPolicy -ExecutionPolicy Unrestricted ```

```powershell
$ cd mozilla-vpn-client
$ conda env create -f env-windows.yml
$ conda activate vpn
# Download the Windows SDK using Xwin
$ ./scripts/windows/conda-setup-xwin-sdk.ps1
# Setup the conda env to use a prebuild Qt from Moz-CI
$ ./scripts/windows/conda_setup_win_qt.ps1
# Reactivate the env to apply the changes:
$ conda deactivate
$ conda activate vpn

# Configure
$ cmake -S . -B build -GNinja

# Compile
$ cmake --build build
```

## Macos 
Get the installer of the latest release [here](https://vpn.mozilla.org/r/vpn/download/macos), you can find past versions on [archive.mozilla.org](https://archive.mozilla.org/pub/vpn/releases/). 

### From Source 

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

```bash
$ cd mozilla-vpn-client
$ conda env create -f env-apple.yml
$ conda activate vpn
$ ./scripts/macos/conda_setup_qt.sh
# Reactivate the env to apply the changes:
$ conda deactivate
$ conda activate vpn

# Configure
$ cmake -S . -B build -GNinja -DCODE_SIGN_IDENTITY=<signing identity>

# Compile
$ cmake --build build
# Run 
$ open ./build/src/Mozilla\ VPN.app
```


Many features of the Mozilla VPN project depend on having a valid code signature. To determine the codesigning identities available to you, you can use the `security` tool on the command line as follows:

```
user@example ~ % security find-identity -v -p codesigning                
  1) AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDD "Apple Development: Jane Doe (XXXXXXXXXX)"
     1 valid identities found
```

Thus, to configure the project to use the above codesigning identity, we can provide the argument `-DCODE_SIGN_IDENTITY=AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDD`

To distribute a signed application for installation on other machines, an
embedded provisioning profile is also required. The path to the provisioning
profile can be specified with the argument
`-DCODE_SIGN_PROFILE=<path/to/embedded.provisionprofile>`.
When this argument is unset, an attempt will be made to automatically lookup the
provisioning profile from the `~/Library/Developer/Xcode/UserData/Provisioning\ Profiles`
directory.

#### Building the installer

Use the `--target macpkg` to build the MacOS installer.

This will produce an macOS installer package at `build-macos/macos/pkg/MozillaVPN.pkg`

#### Building with Xcode

Use the same configure command above and use `-GXcode` as the CMake generator:

```
cmake -S . -B build-macos -DCMAKE_PREFIX_PATH=<Qt unzipped path>/macos/lib/cmake/ -GXcode
```

This will generate an Xcode project file at `build-macos/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

```
open build-macos/Mozilla\ VPN.xcodeproj
```

Once Xcode has opened the project, select the `mozillavpn` target and start the build.


## iOS
Get the latest release [in the Apple App Store](https://apps.apple.com/app/apple-store/id1489407738).

### From Source

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

[!CAUTION]
_Xcode 26 has changed where iOS SDKs are stored. Symbolic links must be created so `qt-cmake` can find the SDK.
(If a future version of `qt-cmake` changes how it looks for iOS SDKs, this section can be removed.)
```bash
$ cd /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/
$ ln -s iPhoneOS.sdk iPhoneOS26.0.sdk # (for iOS 26.0 - change the version number for other SDK versions)
```
*Important: The specific iOS SDK versions must also be downloaded via Xcode. Creating a symbolic link will not download the SDK automatically.*


If the macOS conda environment has not already been installed, install it:
```bash 
$ conda env create -f env-apple.yml -n vpn
$ conda activate vpn
# Setup QT
$ ./scripts/macos/conda_setup_qt.sh
# Check for correctness:
$ which qt-cmake # should be ~/miniconda3/envs/vpn/bin/qt-cmake

qt-cmake -S . -B build-ios \
    -GXcode -DQT_HOST_PATH_CMAKE_DIR=~/Qt/6.10.1/macos/lib/cmake \
    -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS26.0.sdk

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




## WASM

WASM builds allow running the VPN client UI in a browser, primarily for development and testing purposes.
Try it in the [browser](https://mozilla-mobile.github.io/mozilla-vpn-client/)

### From Source

Install [miniconda](https://docs.conda.io/en/latest/miniconda.html), then create and activate the WASM conda environment:

```bash
conda env create -f env-wasm.yml
conda activate vpn
```

Install EMSDK and Qt for WASM:

```bash
./scripts/wasm/conda_setup_emsdk.sh
./scripts/wasm/conda_setup_qt.sh
# Re-activate to apply changes
conda deactivate && conda activate vpn
```

Configure and build:

```bash
qt-cmake -S . -B build-wasm -GNinja \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DBUILD_TESTS=OFF
cmake --build build-wasm
```
You will find the index.html in `<build-folder>/wasm_build/index.html`


## Troubleshooting

## git clean if you're having build troubles
It's easy for your local build to get messed up. When in doubt do a `git clean` to blow away all generated files.
Running `git clean -xfdi .` from the root of the folder provides you an interactive space where you can review everything that's going to be deleted. You may want to exclude `.vscode` or `.env` that you've set-up just the way you like.

## did you activate your conda environment?
You'll need to run `conda activate vpn` (or `vpn-android` for Android) every time you open a new terminal. If you forget and then run cmake the build can get confused / stuck. `git clean`, make sure conda environment is activated, try again.

## ask for help
We are on a mission to make building easy. If you've tried the above two steps, don't get stuck ask for help.

