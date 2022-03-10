# Mozilla VPN

>_One tap to privacy
Surf, stream, game, and get work done while maintaining your privacy online. Whether you’re traveling, using public WiFi, or simply looking for more online security, we will always put your privacy first._ 

See: https://vpn.mozilla.org

## Getting Involved

We encourage you to participate in this open source project. We love Pull Requests, Bug Reports, ideas, (security) code reviews or any other kind of positive contribution. 

Before you attempt to make a contribution please read the [Community Participation Guidelines](https://www.mozilla.org/en-US/about/governance/policies/participation/).

* [View current Issues](https://github.com/mozilla-mobile/mozilla-vpn-client/issues), [view current Pull Requests](https://github.com/mozilla-mobile/mozilla-vpn-client/pulls), or [file a security issue](https://bugzilla.mozilla.org/enter_bug.cgi?product=Mozilla%20VPN).
* Localization happens on [Pontoon](https://pontoon.mozilla.org/projects/mozilla-vpn-client/).
* Matrix [#mozilla-vpn:mozilla.org channel](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org)
* [View the Wiki](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki).
* [View the Support Docs](https://support.mozilla.org/en-US/products/firefox-private-network-vpn)


## Dev instructions

After checking out the code:

* Install the git pre-commit hook (`./scripts/git-pre-commit-format install`)
* Build the source (See below)
* Run the unit tests with `./scripts/tests/unit_tests.sh` or see below for running the functional tests.
* Run the qml tests with `./tests/tests/qml_tests.sh`
* Run the lottie tests with `./tests/tests/lottie_tests.sh`

### Running the functional tests

* Install node (if needed) and then `npm install` to install the testing dependencies
* Install geckodriver and ensure it's on your path.
  [Docs](https://www.selenium.dev/documentation/getting_started/installing_browser_drivers/)
* Make a .env file with:
 * `ACCOUNT_EMAIL` and `ACCOUNT_PASSWORD` (the account should have an active subscription on staging).
 * `MVPN_API_BASE_URL` (where proxy runs, most likely http://localhost:5000)
 * `MVPN_BIN` (location of compiled mvpn binary)
 * `ARTIFACT_DIR` (directory to put screenshots from test failures)
* (Optional) In one window run `./tests/proxy/wsgi.py --mock-devices`
* To run, say, the authentication tests: `./scripts/tests/functional_test.sh tests/functional/testAuthentication.js`.

Misc tips from core devs:
* Make sure there are read/write permissions at every level of your build path
* Suggest building with the flags used in CI `qmake CONFIG+=DUMMY QMAKE_CXX=clang++ QMAKE_LINK=clang++ CONFIG+=debug QT+=svg`
* Using a headless browser locally may cause the tests to stall out.
* See the [workflows file](/.github/workflows/functional_tests.yaml) which runs the functional tests in ci for pointers
  if you're stuck.
* If you're a conda user you can conda install node and geckodriver from conda-forge packages.
* If you're trying to just iterate on one test change `it("....)` to `it.only("...)`. And only that one test will run.... don't forget to undo!
* Check out our [Logviewer](https://mozilla-mobile.github.io/mozilla-vpn-client/logviewer/) and the [Inspector](https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/), they might come handy debugging issues!

## How to build from the source code

### Linux

On linux, the compilation of MozillaVPN is relatively easy. You need the
following dependencies:

- Qt5 >= 5.15.0
- libpolkit-gobject-1-dev >=0.105
- wireguard >=1.0.20200513
- wireguard-tools >=1.0.20200513
- resolvconf >= 1.82
- golang >= 1.13
- python >= 3.6

For Python depedencies see requirements.txt

#### QT5

Qt5 can be installed in a number of ways:
- download a binary package or the installer from the official QT website: https://www.qt.io/download
- use a linux package manager
- compile Qt5 (dinamically or statically).

To build QT5 statically on Ubuntu/20.04, go to the root directory of this project and follow these steps:

```
curl -L https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz --output qt-everywhere-src-5.15.1.tar.xz
tar xvf qt-everywhere-src-5.15.1.tar.xz
mv qt-everywhere-src-5.15.1 qt
sudo apt build-dep qt5-default
sudo apt install clang llvm
sudo apt install libxcb-xinerama0-dev libxcb-util-dev
bash scripts/utils/qt5_compile.sh qt qt
```

See https://wiki.qt.io/Building_Qt_5_from_Git#Linux.2FX11 if you get stuck or are on another distro.

If you are building Qt from sources, be sure to **add `$(pwd)/qt/qt/bin` to `PATH`.**

For Ubuntu/21.04 and later, the `qt5-default` package has been deprecated, but Qt 5.15 is now
available as a native package. For these distributions, you can install the necessary Qt
dependencies as follows:
```
sudo apt-get install devscripts equivs
sudo mk-build-deps --install linux/debian/control.$(lsb_release -sc)
sudo apt-get install qml-module-qtcharts qml-module-qtgraphicaleffects \
                     qml-module-qtquick-controls qml-module-qtquick-controls2 \
                     qml-module-qtquick-extras qml-module-qtquick-layouts \
                     qml-module-qtquick-window2 qml-module-qtquick2 \
                     qml-module-qtqml-models2 qml-module-qtqml
```

#### Initialization

```
# submodules
git submodule init
git submodule update
# glean
./scripts/utils/generate_glean.py
# translations
./scripts/utils/import_languages.py
# Bake shaders (qt6 only)
sh ./scripts/utils/bake_shaders.sh
```

#### Build

To build next to source:

```
make -j8 # replace 8 with the number of cores. Or use: make -j$(nproc)
sudo make install
```

For local dev builds, the following qmake command may be more helpful `qmake CONFIG+=debug`.

If you prefer to not install at /usr or /etc, you can specify alternate prefixes. Using no prefixes is equivalent to:

```
qmake USRPATH=/usr ETCPATH=/etc
```

#### Run

If you have built into /usr, simply run

```
mozillavpn
```

Alternatively, you can use two terminals to run the daemon manually and seperately e.g.

```
sudo mozillavpn linuxdaemon
mozillavpn
```

mozillavpn linuxdaemon needs privileged access and so if you do not run as root, you will get an authentication prompt every time you try to reconnect the vpn.


### MacOS

On macOS, we strongly suggest to compile Qt5 statically. To do that, follow these steps:

```
curl -L https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz --output qt-everywhere-src-5.15.1.tar.xz
tar vxf qt-everywhere-src-5.15.1.tar.xz
mv qt-everywhere-src-5.15.1 qt
bash scripts/utils/qt5_compile.sh `pwd`/qt qt
export QT_MACOS_BIN=`pwd`/qt/qt/bin
```

The procedure to compile MozillaVPN for macOS is the following:

1. Install XCodeProj:
```
  $ [sudo] gem install xcodeproj
```
2. Install go v1.16+ if you haven't done it before: https://golang.org/dl/

Some developers have experienced that in step 8 (when you're in XCode) that XCode reports that 
go isn't available and so you can't build the app and dependencies in XCode. 
In this case, a workaround is to symlink go into XCode directory as follows:

* Make sure go is 1.16+: `go version`
* Find the location of go binary `which go` example output `/usr/local/go/bin/go`
* Symlink e.g. `sudo ln -s /usr/local/go/bin/go /Applications/Xcode.app/Contents/Developer/usr/bin/go` 

This step needs to be updated each time XCode updates.

3. Update the submodules:
```
  $ git submodule init
  $ git submodule update
```
4. Install python3 dependencies:
```
  $ pip3 install -r requirements.txt --user
```
5. Copy `xcode.xconfig.template` to `xcode.xconfig`
```
  $ cp xcode.xconfig.template xcode.xconfig
```
6. Modify xcode.xconfig to something like:
```
DEVELOPMENT_TEAM = 43AQ936H96

# MacOS configuration
GROUP_ID_MACOS = group.org.mozilla.macos.Guardian
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item

# IOS configuration
GROUP_ID_IOS = <>
APP_ID_IOS = <>
NETEXT_ID_IOS = <>
```
7. Install rust: https://www.rust-lang.org/
8. Run the script (use QT\_MACOS\_BIN env to set the path for the Qt5 macos build bin folder):
```
  $ ./scripts/macos/apple_compile.sh macos
```
9. Xcode should automatically open. You can then run/test/archive/ship the app

To build a Release style build (ready for signing), use:
```
cd MozillaVPN.xcodeproj
xcodebuild -scheme MozillaVPN -workspace project.xcworkspace -configuration Release clean build CODE_SIGNING_ALLOWED=NO
```

The built up will show up in `Release/Mozilla VPN.app` (relative to the root of the repo).

### IOS

For IOS, we recommend installing Qt using the [Qt Online Installer](https://www.qt.io/download-qt-installer). We recommend installing
the latest available release of Qt5 for IOS. You may also need to enable support for Qt Charts and Qt Network Qt Network Authorization
during the installation.

Once Qt has been installed, the IOS procedure is similar to the macOS one:

1. Install XCodeProj:
```
  $ [sudo] gem install xcodeproj
```

2. Update the submodules:
```
  $ git submodule init
  $ git submodule update
```

3. Install python3 dependencies:
```
  $ pip3 install -r requirements.txt --user
```

4. Copy `xcode.xconfig.template` to `xcode.xconfig`
```
  $ cp xcode.xconfig.template xcode.xconfig
```

5. Modify xcode.xconfig to something like:
```
DEVELOPMENT_TEAM = 43AQ936H96

# MacOS configuration
GROUP_ID_MACOS = <>
APP_ID_IOS = <>
NETEXT_ID_IOS = <>
LOGIN_ID_IOS = <>

# IOS configuration
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
```
6. Run the script (use QT\_IOS\_BIN env to set the path for the Qt5 ios build bin folder):
```
  $ ./scripts/macos/apple_compile.sh ios
```
Add the Adjust SDK token with `-a | --adjust <adjust_token>`

7. Open Xcode and run/test/archive/ship the app

### Android 

1. Install go if you haven't done it before: https://golang.org/dl/
2. Install Android SDK/NDK + JDK - https://doc.qt.io/qt-5/android-getting-started.html
3. We currently require NDK r20b and SDK >=21
4. Update the submodules:
```bash 
  $ git submodule init
  $ git submodule update
```

5. Install python3 dependencies:
```
  $ pip3 install -r requirements.txt --user
```

6. Build the apk
```bash 
  $  ./scripts/android/package.sh /path/to/Qt/5.15.x/ (debug|release)
```
Add the Adjust SDK token with `-a | --adjust <adjust_token>`
7. The apk will be located in ```.tmp/src/android-build//build/outputs/apk/debug/android-build-debug.apk```
8. Install with adb on device/emulator
```bash
  $ adb install .tmp/src/android-build//build/outputs/apk/debug/android-build-debug.apk
```

### Windows

We use a statically-compiled QT5.15 version to deploy the app. There are many
tutorials about to how to compile QT5 on windows, but to make this task
easier for everyone, there is a batch script to execute into a visual-studio
x86 context: `$ scripts\qt5_compile.bat`

The dependencies are:
1. perl: http://strawberryperl.com/
2. nasm: https://www.nasm.us/
3. python3: https://www.python.org/downloads/windows/
4. visual studio 2019: https://visualstudio.microsoft.com/vs/
5. Install python3 dependencies (pip3 install -r requirements.txt --user)
6. Install rust: https://www.rust-lang.org/

Openssl can be obtained from here: https://www.openssl.org/source/
Qt5.15 can be obtained from: https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz

There is also a script to compile the application: `scripts\windows_compile.bat`

## Developer Options and staging environment

To enable the staging environment, open the `Get Help` window, and click on the `Get Help` text 6 times within 10
seconds to unlock the Developer Options menu. On this menu, you can enable on the `Staging Server` checkbox to
switch to the staging environment. A full restart of the VPN will be required for this option to take effect.

## Inspector

The inspector is enabled when the staging environment is activated.
When running MozillaVPN, go to https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/ to view the inspector.
From this page, connect it to ws://localhost:8765

From the inspector, type `help` to see the list of available commands.

## Glean

When the client is built in debug mode, pings will have the applicationId `MozillaVPN-debug`. Additionally, ping contents will be logged to the client logs and will also be sent to the
[glean debug viewer](https://debug-ping-preview.firebaseapp.com/pings/MozillaVPN) (login required) where they are retained for 3 weeks.

More info on debug view in [glean docs](https://mozilla.github.io/glean/book/user/debugging/index.html).

When the client is in staging mode, but not debug mode, pings will have the applicationId `MozillaVPN-staging` which allows for filtering between staging and production pings.

#### A note on glean embedding

Qt only accepts `major.minor` versions for importing. So if, for example, you're embedding glean v0.21.2 then it will still, for Qt's purpose, be v0.21.

## Bug report

Please file bugs here: https://github.com/mozilla-mobile/mozilla-vpn-client/issues

## Status

[![Unit Test Coverage](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_coverage.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_coverage.yaml)
[![Functional tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/functional_tests.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/functional_tests.yaml)
[![Linters (clang, l10n)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml)
[![Android](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/android.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/android.yaml)
[![MacOS](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml)
[![Windows](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml)
