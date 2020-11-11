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

#### Building qt5

Qt5 can be installed in a number of ways. To build statically on Ubuntu, follow these steps:

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
make -jN  # N cores
make install
```

To build in /usr:

```
qmake PREFIX=/usr
make -jN  # N cores
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
xcodebuild -scheme Mozilla -workspace project.xcworkspace -configuration Release clean build CODE_SIGNING_ALLOWED=NO
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

### Other platforms

We are working on Android and Windows.

## Bug report

Please file bugs here: https://github.com/mozilla-mobile/mozilla-vpn-client/issues
