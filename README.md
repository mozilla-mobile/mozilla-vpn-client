# Mozilla VPN

>_One tap to privacy_
>
>_Surf, stream, game, and get work done while maintaining your privacy online. Whether you’re traveling, using public WiFi, or simply looking for more online security, we will always put your privacy first._

See https://vpn.mozilla.org for more details.

## Getting involved

We encourage you to participate in this open source project. We love pull requests, bug reports, ideas, (security) code reviews or any other kind of positive contribution.

Before you attempt to make a contribution please read the [Community Participation Guidelines](https://www.mozilla.org/en-US/about/governance/policies/participation/).

* [View current issues](https://github.com/mozilla-mobile/mozilla-vpn-client/issues), [view current pull requests](https://github.com/mozilla-mobile/mozilla-vpn-client/pulls), or [file a security issue](https://bugzilla.mozilla.org/enter_bug.cgi?product=Mozilla%20VPN)
* Localization happens on [Pontoon](https://pontoon.mozilla.org/projects/mozilla-vpn-client/)
* Discuss on the [Matrix channel](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org)
* View the [wiki](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki)
* View the [support docs](https://support.mozilla.org/en-US/products/firefox-private-network-vpn)

## Installation and usage

### Quick start

1. Clone this repository
2. Checkout submodules: `git submodule init && git submodule update`
3. Install pip dependencies: `pip3 install -r requirements.txt`
4. Install the Git pre-commit hook: `scripts/git-pre-commit-format install`
5. Build the source for your OS: [Linux](), [MacOS](), [iOS](), [Android](), [Windows]()
6. Run the tests: `scripts/test_coverage.sh && scripts/test_function.sh`


## Building from source

Mozilla VPN depends on Qt 5.15.2. In general, we recommend compiling Qt statically, but it is also possible to install using the [online installer](https://www.qt.io/download). Instructions and helper scripts for compilation on each OS are provided below.


### Linux

#### Software requirements

TODO

#### Building Qt

TODO

```bash
# Install dependencies
sudo apt build-dep qt5-default
sudo apt install clang llvm libxcb-xinerama0-dev libxcb-util-dev

# Download and unpack the Qt source files
curl -L https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz \
  --o qt-everywhere-src-5.15.2.tar.xz
tar xvf qt-everywhere-src-5.15.2.tar.xz

# Build Qt
scripts/qt5_compile.sh qt-everywhere-src-5.15.2 qt

# Cleanup
rm -rf qt-everywhere-src-5.15.2.tar.xz qt-everywhere-5.15.2
```

Finally, add the Qt bin dir to the PATH:

```bash
export PATH=$(pwd)/qt/bin:$PATH
```

#### Building the application

TODO

```bash
qmake CONFIG+=debug
make -j$(nproc)
sudo make install
```

Then run `mozillavpn`.

Alternatively, you can use two terminals to run the daemon manually and separately:

```bash
sudo mozillavpn linuxdaemon
mozillavpn
```

`mozillavpn linuxdaemon` needs privileged access and so if you do not run as root, you will get an authentication prompt every time you try to reconnect the vpn.

### MacOS

#### Software requirements

1. Install [Xcode](https://developer.apple.com/xcode/)
2. Install Xcodeproj: `[sudo] gem install xcodeproj`
3. Install [Go](https://golang.org/dl/) >= 1.16

Some developers have experienced a problem where Xcode reports that Go isn't available, which prevents the app from being built. If this causes problems, a simple workaround is to symlink the go binary into a location where Xcode can see it:

```bash
sudo ln -s $(which go) /Applications/Xcode.app/Contents/Developer/usr/bin/go
```

This step will need to be repeated if Xcode is updated.

#### Building Qt

TODO header

```bash
# Download and unpack the Qt source files
curl -L https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz \
  --o qt-everywhere-src-5.15.2.tar.xz
tar xvf qt-everywhere-src-5.15.2.tar.xz

# Build Qt
scripts/qt5_compile.sh qt-everywhere-src-5.15.2 qt

# Cleanup
rm -rf qt-everywhere-src-5.15.2.tar.xz qt-everywhere-5.15.2
```

#### Building the application

Copy `xcode.xconfig.template` to `xcode.xconfig`. You may need to tweak the default entries, but for the majority of cases they should work as-is.

(TODO verify this change makes sense)

```bash
cp xcode.xconfig.template xcode.xconfig
```

Then run the compilation script. (The script below will work as-is, but you may also be interested in flags like `-i` for the inspector. Run `scripts/apple_compile.sh --help` to see all options.)

```bash
QT_MACOS_BIN=$(pwd)/qt/bin scripts/apple_compile.sh macos
```

Xcode should automatically open, which you can use to run, test, archive or ship the app.

To build a release-style build (ready for signing), use:

```bash
cd MozillaVPN.xcodeproj
xcodebuild -scheme MozillaVPN -workspace project.xcworkspace -configuration Release clean build CODE_SIGNING_ALLOWED=NO
```

### iOS

TODO

### Android

#### Software requirements

1. Install [Go](https://golang.org/dl/) >= 1.16
2. Install Android Tools:
  - Android SDK >= 21
  - Android NDK (version r20b)
  - Java Development Kit (JDK)

Android tools can be installed by following these [instructions from Qt](https://doc.qt.io/qt-5/android-getting-started.html) or by installing [Android Studio](https://developer.android.com/studio).

#### Building Qt

Follow the instructions for building Qt on your operating system: [Linux](), [MacOS](), [Windows]().

#### Building the application

Build the APK:

```bash
scripts/android_package.sh /path/to/qt -d
```

A few notes on flags:

- The `-d` flag will build the VPN in debug mode. To build in release mode, omit the `-d` flag.
- An Adjust SDK token can optionally be added by passing `--adjust <adjust_token>`.

The built APK will be located in one of the following locations:

- **debug**: `.tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk`
- **release**: `.tmp/src/android-build/build/outputs/apk/release/android-build-universal-release-unsigned.apk`

Install the APK on a device or emulator:

```bash
adb install .tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk
```

### Windows

#### Software requirements

1. Install [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) and download these VS2019 components:
  - The "Desktop development with C++" workload
  - The "C++ 2019 Redistributable MSMs" component
2. Install the following packages and make them available on your `PATH`:
  - [perl](http://strawberryperl.com/)
  - [nasm](https://www.nasm.us/)
  - [python3](https://www.python.org/downloads/windows/)

_Note that unlike other operating systems, Python is not bundled with Windows, and must be installed prior to installing the pip dependencies listed in the [quick start]() section._

#### Building Qt

Download the following source packages and unpack them into a directory (the recommended location is `C:\MozillaVPNBuildDeps`):

- [OpenSSL 3.0](https://www.openssl.org/source/openssl-3.0.0.tar.gz)
- [Qt 5.15.2](https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz)

Qt must be built in a Visual Studio 2019 context. A convenience script, `qt5_compile.bat`, exists to facilitate this process. First, copy `env.bat.template` to `env.bat`. This file will be read in by the compile script and can be used to configure the environment. The default values should work in most instances, but if you've unpacked your dependencies into a different directory, the values will need to be changed to match.

Then, run the script with the paths to OpenSSL and Qt as arguments:

```batch
.\scripts\qt5_compile.bat C:\MozillaVPNBuildDeps\openssl-3.0.0 C:\MozillaVPNBuildDeps\qt-everywhere-src-5.15.2
```

#### Building the application

If Qt compiled correctly, building the app is as simple as running `.\scripts\windows_compile.bat`. (Note that this script also makes use of the `env.bat` file configured in the last step.) The built application will appear in the project root directory.


## "Developer Options" and the staging environment

To enable the staging environment, open the `Get Help` window, and click on the `Get Help` text 6 times within 10
seconds to unlock the Developer Options menu. On this menu, you can enable on the `Staging Server` checkbox to
switch to the staging environment. A full restart of the VPN will be required for this option to take effect.

## Inspector

The inspector is enabled when the staging environment is activated.
When running MozillaVPN, go to http://localhost:8766 to view the inspector.

From the inspector, type `help` to see the list of available commands.

## Bug report

Please file bugs here: https://github.com/mozilla-mobile/mozilla-vpn-client/issues

## Status

[![Unit Test Coverage](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_coverage.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_coverage.yaml)
[![Functional tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/functional_tests.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/functional_tests.yaml)
[![Linters (clang, l10n)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml)
[![Android](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/android.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/android.yaml)
[![MacOS](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml)
[![Windows](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml)

# README TODOs

- Add links to quick start
- Add anchor links to windows build
- Test build instructions from scratch
- Building android on windows?
- Windows package managers (chocolatey, etc)
- The original README said, re: `xcode.xconfig.template`, "modify xcode.config to something like:" ... why "something like"? Why would this ever need to change?
