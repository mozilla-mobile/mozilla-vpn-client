# Mozilla VPN

See: https://vpn.mozilla.org

## Dependencies

MozillaVPN requires Qt5 (5.15 or higher)

## How to build from the source code

### Linux

On linux, the compilation of MozillaVPN is relative easy. You need the
following dependencies:

- Qt5 >= 5.15.0
- libpolkit-gobject-1-0 >=0.105
- wireguard >=1.0.20200513
- wireguard-tools >=1.0.20200513
- resolvconf >= 1.82

The procedure is the following:
1. Update the submodules:
  $ git submodule init
  $ git submodule update --remote
2. Compile:
  $ qmake PREFIX=/usr
  $ make -j4
3. Install:
  $ sudo make install
4. Run:
  $ mozillavpn

The installation phase is important because mozillavpn needs to talk with
mozillavpn-daemon (see the code in linux/daemon) via DBus.

### MacOS

On macOS, we strongly suggest to compile Qt5 statically. To do that, use:
```
curl https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz > qt.tar.xz
tar -jvxf qt.tar.xz
mv qt-everywhere-src-5.15.1 qt
bash scripts/qt5_compile.sh `pwd`/qt qt
export QT_MACOS_BIN=`pwd`/qt/qt/bin
```

The procedure to compile MozillaVPN for macOS is the following:

1. Install XCodeProj:
  $ [sudo] gem install xcodeproj
2. Update the submodules:
  $ git submodule init
  $ git submodule update --remote
3. Run the script (use QT\_MACOS\_BIN env to set the path for the Qt5 macos build bin folder):
  $ ./scripts/apple\_compile.sh macos
4. Copy `xcode.xconfig.template` to `xcode.xconfig`
  $ cp xcode.xconfig.template xcode.xconfig
5. Modify xcode.xconfig to something like:
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
6. Open Xcode and run/test/archive/ship the app

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
2. Update the submodules:
  $ git submodule init
  $ git submodule update --remote
4. Copy `xcode.xconfig.template` to `xcode.xconfig`
  $ cp xcode.xconfig.template xcode.xconfig
5. Modify xcode.xconfig to something like:
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
5. Run the script (use QT\_IOS\_BIN env to set the path for the Qt5 ios build bin folder):
  $ ./scripts/apple\_compile.sh ios
6. Open Xcode and run/test/archive/ship the app

### Other platforms

We are working on Android and Windows.

## Bug report

Please file bugs here: https://github.com/mozilla-mobile/mozilla-vpn-client/issues
