# Mozilla VPN

>_One tap to privacy
Surf, stream, game, and get work done while maintaining your privacy online.
Whether you’re traveling, using public WiFi, or simply looking for more online
security, we will always put your privacy first._

See: https://vpn.mozilla.org

## Getting Involved

We encourage you to participate in this open source project. We love Pull
Requests, Bug Reports, ideas, (security) code reviews, or any other kind of
positive contribution.

Before you attempt to make a contribution please read the [Community
Participation
Guidelines](https://www.mozilla.org/en-US/about/governance/policies/participation/).

Here are some useful links to start:

* [View open
  issues](https://github.com/mozilla-mobile/mozilla-vpn-client/issues)
* [View open pull
  requests](https://github.com/mozilla-mobile/mozilla-vpn-client/pulls)
* [File an
   issue](https://github.com/mozilla-mobile/mozilla-vpn-client/issues/new/choose)
* [File a security
  issue](https://bugzilla.mozilla.org/enter_bug.cgi?product=Mozilla%20VPN)
* Join the [matrix
  channel](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org)
* [View the wiki](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki)
* [View the support
  docs](https://support.mozilla.org/en-US/products/firefox-private-network-vpn)
* Localization happens on
  [Pontoon](https://pontoon.mozilla.org/projects/mozilla-vpn-client/).

If you want to submit a pull-request, please, install the clang format
pre-commit hook: `./scripts/git-pre-commit-format install`

## Checking out the source code

```bash
git clone https://github.com/mozilla-mobile/mozilla-vpn-client.git
cd mozilla-vpn-client
git submodule update --init
```

## How to build from the source code

In order to build this application, you need to install a few dependencies.

#### Install Qt6

Qt6 can be installed in a number of ways:

- download a binary package or the installer from the official QT website:
  https://www.qt.io/download - this is the recommended way for Android and iOS
  builds.
- use a package manager. For instance, we use
  [aqt](https://github.com/miurahr/aqtinstall) for WASM builds.
- compile Qt6 (dynamically or statically). If you want to choose this path, you
  can use our bash script for macOS and Linux:
```bash
./scripts/utils/qt6_compile.sh </qt6/source/code/path> </destination/path>
```
   ... or our batch script for windows:
```bash
./scripts/Qt5_static_compile.bat
```

#### Install Python 3

[Python](https://www.python.org/) >= 3.6 is required. You also need to install
a few python modules using [pip](https://pypi.org/):

```bash
pip install -r requirements.txt --user
```

#### Install rust

[Rust](https://www.rust-lang.org/) is required for desktop builds (MacOS, Linux
and Windows). See the official rust documentation to know how to install it.

#### What's next?

We support the following platforms: Linux, Windows, MacOS, iOS, Android and
WASM. Each one is unique and it has a different section in this document.

### How to build from source code on Linux

We have tested Mozilla VPN on Ubuntu, Fedora, and Arch Linux but in this
document, we focus on Ubuntu only.

1. On Ubuntu, the compilation of MozillaVPN is relatively easy. In addition to
what we wrote before, you also need the following dependencies:

- libpolkit-gobject-1-dev >= 0.105
- wireguard >= 1.0.20200513
- wireguard-tools >= 1.0.20200513
- resolvconf >= 1.82
- golang >= 1.13

2. Instrument the glean events:
```bash
./scripts/utils/generate_glean.py
```

3. Import the languages and generate the translation strings:
```bash
./scripts/utils/import_languages.py
```

4. **Optional**: In case you want to change the shaders, you must regenerate
them:
```bash
./scripts/utils/bake_shaders.sh
```

5. Finally, we are able to configure the whole project using `qmake`. Usually,
`qmake` is already in your path, but if it's not, add the Qt6 installation path
in your `PATH` env variable.
```bash
qmake # for local dev builds use `qmake CONFIG+=debug`
```
If you prefer to not install at /usr or /etc, you can specify alternate
prefixes. Using no prefixes is equivalent to:
```bash
qmake USRPATH=/usr ETCPATH=/etc
```

6. Compile the source code:
```bash
make -j8 # replace 8 with the number of cores. Or use: make -j$(nproc)
```

7. Installation:
```bash
sudo make install
```

8.  After the installation, you can run the app simply running:
```bash
mozillavpn
```

Alternatively, you can use two terminals to run the daemon manually and
separately e.g.

```bash
sudo mozillavpn linuxdaemon
mozillavpn
```

mozillavpn linuxdaemon needs privileged access and so if you do not run as
root, you will get an authentication prompt every time you try to reconnect the
vpn.

### How to build from source code on MacOS

1. On MacOS, we compile the app using
[XCode](https://developer.apple.com/xcode/) version 12 or higher.

2. We use `qmake` to generate the XCode project and then we "patch" it to add
extra components such as the wireguard, the browser bridge and so on. We patch
the XCode project using [xcodeproj](https://github.com/CocoaPods/Xcodeproj). To
install it:
```bash
gem install xcodeproj # probably you want to run this command with `sudo`
```

3. You also need to install go >= v1.16. If you don't have it done already,
download go from the [official website](https://golang.org/dl/).

4. Copy `xcode.xconfig.template` to `xcode.xconfig`
```bash
cp xcode.xconfig.template xcode.xconfig
```

5. Modify xcode.xconfig to something like:
```
# MacOS configuration
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item

# IOS configuration
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
```

6. Generate the XCode project using our script:
```bash
./scripts/macos/apple_compile.sh macos
```

If `qmake` cannot be found in your `$PATH`, run this script using
QT\_MACOS\_BIN env to set the path for the Qt6 macos build bin folder.

7. Xcode should automatically open. You can then run/test/archive/ship the app.
If you prefer to compile the app in command-line mode, use the following
command:
```bash
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project "Mozilla VPN.xcodeproj"
```

The built up will show up in `Release/Mozilla VPN.app` (relative to the root of the repo).

*Note*: some developers have experienced that XCode reports that go isn't
available and so you can't build the app and dependencies in XCode.  In this
case, a workaround is to symlink go into XCode directory as follows:

* Make sure go is 1.16+: `go version`
* Find the location of go binary `which go` example output `/usr/local/go/bin/go`
* Symlink e.g.
```bash
sudo ln -s /usr/local/go/bin/go /Applications/Xcode.app/Contents/Developer/usr/bin/go
```

This step needs to be executed each time XCode updates.

### How to build from source code for iOS

Follow the same steps as for MacOS. When you are about to generate the XCode
project, run this command instead:
```bash
./scripts/macos/apple_compile.sh ios
```
Add the Adjust SDK token with `-a | --adjust <adjust_token>`

### How to build from source code for Android

1. You need to install go >= v1.16. If you don't have it done already, download
go from the [official website](https://golang.org/dl/).

2. Follow the [Getting
started](https://doc.qt.io/qt-6/android-getting-started.html) page.

3. Build the apk
```bash
./scripts/android/package.sh </path/to/Qt6/> <debug|release>
```
Add the Adjust SDK token with `-a | --adjust <adjust_token>`

4. The apk will be located in
`.tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk`

5. Install with adb on device/emulator
```bash
adb install .tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk
```

### How to build from source code for Windows

For Windows, there are a few extra dependencies to install:

1. perl: http://strawberryperl.com/
2. nasm: https://www.nasm.us/
3. visual studio 2019: https://visualstudio.microsoft.com/vs/
4. OpenSSL: https://www.openssl.org/source/

We use a script to compile the app for windows and to generate the installer.
```
scripts\windows\compile.bat
```

### How to build from source code for WASM

Mozilla VPN can be used as a WASM module to test the UI, the localization and
to simulate issues. Take a look at our WASM build
[here](https://mozilla-mobile.github.io/mozilla-vpn-client/).

To build the app as a WASM module, you must install
[emscripten](https://emscripten.org/) and Qt6 for wasm.

Read the following pages to know more:
- https://emscripten.org/docs/getting_started/index.html
- https://doc.qt.io/qt-6/wasm.html

When you are ready, use this script to generate the build:
```bash
scripts/wasm/compile.sh
```

## Testing

* Run the unit tests with `./scripts/tests/unit_tests.sh`
* Run the qml tests with `./tests/tests/qml_tests.sh`
* Run the lottie tests with `./tests/tests/lottie_tests.sh`
* Run the funcional tests (See below)

### Running the functional tests

* Install node (if needed) and then `npm install` to install the testing
  dependencies
* Install geckodriver and ensure it's on your path.
  [Docs](https://www.selenium.dev/documentation/getting_started/installing_browser_drivers/)
* Make a .env file with:
 * `ACCOUNT_EMAIL` and `ACCOUNT_PASSWORD` (the account should have an active
   subscription on staging).
 * `MVPN_API_BASE_URL` (where proxy runs, most likely http://localhost:5000)
 * `MVPN_BIN` (location of compiled mvpn binary)
 * `ARTIFACT_DIR` (directory to put screenshots from test failures)
* (Optional) In one window run `./tests/proxy/wsgi.py --mock-devices`
* To run, say, the authentication tests: `./scripts/tests/functional_test.sh
  tests/functional/testAuthentication.js`.

## Developer Options and staging environment

To enable the staging environment, open the `Get Help` window, and click on the
`Get Help` text 6 times within 10 seconds to unlock the Developer Options menu.
On this menu, you can enable on the `Staging Server` checkbox to switch to the
staging environment. A full restart of the VPN will be required for this option
to take effect.

## Inspector

The inspector is a debugging tool available only when the staging environment
is activated. When running MozillaVPN, go to the [inspector page](
https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/) to interact
with the app. Connect the inspector to the app using the web-socket interface.
On desktop, use `ws://localhost:8765`.

From the inspector, type `help` to see the list of available commands.

## Glean

[Glean](https://docs.telemetry.mozilla.org/concepts/glean/glean.html) is a
Mozilla new product analytics & telemetry solution that provides a consistent
experience and behavior across all of Mozilla products.

When the client is built in debug mode, pings will have the applicationId
`MozillaVPN-debug`. Additionally, ping contents will be logged to the client
logs and will also be sent to the
[glean debug
viewer](https://debug-ping-preview.firebaseapp.com/pings/MozillaVPN) (login
required) where they are retained for 3 weeks.

More info on debug view in [glean
docs](https://mozilla.github.io/glean/book/user/debugging/index.html).

When the client is in staging mode, but not debug mode, pings will have the
applicationId `MozillaVPN-staging` which allows for filtering between staging
and production pings.

#### A note on glean embedding

Qt only accepts `major.minor` versions for importing. So if, for example,
you're embedding glean v0.21.2 then it will still, for Qt's purpose, be v0.21.

## Status

[![Unit Tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_unit.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_unit.yaml)
[![QML Tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_qml.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_qml.yaml)
[![Lottie Tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_lottie.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_lottie.yaml)
[![Linters (clang, l10n)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml)
[![Linux Packages](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linux.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linux.yaml)
[![MacOS](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml)
[![WebAssembly](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/wasm.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/wasm.yaml)
[![Windows](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml)
[![iOS](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/ios-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/ios-build.yaml)
