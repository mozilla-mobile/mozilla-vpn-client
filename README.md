# Mozilla VPN

See: https://vpn.mozilla.org

## Dependencies

MozillaVPN requires Qt5 (5.15 or higher)

## How to build from the source code

### Linux

On linux, the compilation of MozillaVPN is relative easy. You need the
following dependencies:

- Qt5 >= 5.15.0
- libpolkit-gobject-1-dev >=0.105
- wireguard >=1.0.20200513
- wireguard-tools >=1.0.20200513
- resolvconf >= 1.82

#### QT5

Qt5 can be installed in a number of ways:
- download a binary package or the installer from the official QT website: https://www.qt.io/download
- use a linux package manager
- compile Qt5 (dinamically or statically).

To build QT5 statically on Ubuntu/Debian, go to the root directory of this project and follow these steps:

```
curl -L https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz --output qt-everywhere-src-5.15.1.tar.xz
tar xvf qt-everywhere-src-5.15.1.tar.xz
mv qt-everywhere-src-5.15.1 qt
sudo apt install sudo clang build-dep qt5-default libxcb-xinerama0-dev
bash scripts/qt5_compile.sh qt qt
```

See https://wiki.qt.io/Building_Qt_5_from_Git#Linux.2FX11 if you get stuck or are on another distro.

Finally, **add `$(pwd)/qt/qt/bin` to `PATH`.**

#### Initialize submodules

```
git submodule init
git submodule update --remote
```

This may result in submodule updates that need to be checked in

#### Build

To build next to source:

```
mkdir build
qmake PREFIX=$(pwd)/build
make -j8 # replace 8 with the number of cores. Or use: make -j$(nproc)
make install
```

To build in /usr:

```
qmake PREFIX=/usr
make -j8 # replace 8 with the number of cores. Or use: make -j$(nproc)
sudo make install  # Must use sudo to install in /usr
```

#### Run

If you have built into /usr, simply run

```
mozillavpn
```

If you have built in `build` directory, open two terminals

```
cd build/bin
sudo ./mozillavpn-daemon
./mozillavpn
```

mozillavpn-daemon needs privileged access and so if you do not run as root, you will get an authentication prompt every time you try to reconnect the vpn.


### MacOS

On macOS, we strongly suggest to compile Qt5 statically. To do that, follow these steps:

```
curl -L https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz --output qt-everywhere-src-5.15.1.tar.xz
tar vxf qt-everywhere-src-5.15.1.tar.xz
mv qt-everywhere-src-5.15.1 qt
bash scripts/qt5_compile.sh `pwd`/qt qt
export QT_MACOS_BIN=`pwd`/qt/qt/bin
```

The procedure to compile MozillaVPN for macOS is the following:

1. Install XCodeProj:
  $ [sudo] gem install xcodeproj
1. Install go if you haven't done it before: https://golang.org/dl/
1. Update the submodules:
  $ git submodule init
  $ git submodule update --remote
1. Run the script (use QT\_MACOS\_BIN env to set the path for the Qt5 macos build bin folder):
  $ ./scripts/apple\_compile.sh macos
1. Copy `xcode.xconfig.template` to `xcode.xconfig`
  $ cp xcode.xconfig.template xcode.xconfig
1. Modify xcode.xconfig to something like:
```
DEVELOPMENT_TEAM = 43AQ936H96

# MacOS configuration
GROUP_ID_MACOS = <>
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login

# IOS configuration
GROUP_ID_IOS = <>
APP_ID_IOS = <>
NETEXT_ID_IOS = <>
```
1. Open Xcode and run/test/archive/ship the app

To build a Release style build (ready for signing), use:
```
cd MozillaVPN.xcodeproj
xcodebuild -scheme MozillaVPN -workspace project.xcworkspace -configuration Release clean build CODE_SIGNING_ALLOWED=NO
```

The built up will show up in `Release/Mozilla VPN.app` (relative to the root of the repo).

### IOS

The IOS procedure is similar to the macOS one:
1. Install XCodeProj:
  $ [sudo] gem install xcodeproj
1. Update the submodules:
  $ git submodule init
  $ git submodule update --remote
1. Copy `xcode.xconfig.template` to `xcode.xconfig`
  $ cp xcode.xconfig.template xcode.xconfig
1. Modify xcode.xconfig to something like:
```
DEVELOPMENT_TEAM = 43AQ936H96

# MacOS configuration
GROUP_ID_MACOS = <>
APP_ID_IOS = <>
NETEXT_ID_IOS = <>
LOGIN_ID_IOS = <>

# IOS configuration
GROUP_ID_IOS = <>
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
```
1. Run the script (use QT\_IOS\_BIN env to set the path for the Qt5 ios build bin folder):
  $ ./scripts/apple\_compile.sh ios
1. Open Xcode and run/test/archive/ship the app

### Android 

1. Install go if you haven't done it before: https://golang.org/dl/
2. Install Android SDK/NDK + JDK - https://doc.qt.io/qt-5/android-getting-started.html
3. We currently require NDK r20b and SDK >=21
4. Update the submodules:
```bash 
  $ git submodule init
  $ git submodule update --remote
```

5. Build the apk
```bash 
  $  ./scripts/android_package.sh /path/to/Qt/5.15.x/ (debug|release)
```
6. The apk will be located in ```.tmp/src/android-build//build/outputs/apk/debug/android-build-debug.apk```
7. Install with adb on device/emulator
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

Openssl can be obtained from here: https://www.openssl.org/source/
Qt5.15 can be obtained from: https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz

There is also a script to compile the application: `scripts\windows_compile.bat`

## Bug report

Please file bugs here: https://github.com/mozilla-mobile/mozilla-vpn-client/issues
