# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

CONFIG += c++1z
# No QT for this project
CONFIG -= qt
CONFIG += console
TEMPLATE  = app
TARGET = mozillavpnnp

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

# For MacOS, please keep the list of source files in sync with
# `scripts/xcode_patcher.rb`.

SOURCES += \
        handler.cpp \
        logger.cpp \
        main.cpp \
        vpnconnection.cpp

HEADERS += \
        handler.h \
        logger.h \
        vpnconnection.h \
        json.hpp

linux:!android {
    target.path = /usr/lib/mozillavpn
    INSTALLS += target

    manifestFirefox.path = /usr/lib/mozilla/native-messaging-hosts
    manifestFirefox.files = manifests/linux/mozillavpn.json
    INSTALLS += manifestFirefox

    manifestChrome.path = /etc/opt/chrome/native-messaging-hosts
    manifestChrome.files = manifests/linux/mozillavpn.json
    INSTALLS += manifestChrome

    manifestChromium.path = /etc/chromium/native-messaging-hosts
    manifestChromium.files = manifests/linux/mozillavpn.json
    INSTALLS += manifestChromium
} else:win* {
    CONFIG(release, debug|release) {
        QMAKE_CXXFLAGS += -MP -Zc:preprocessor
    } else {
        QMAKE_CXXFLAGS += /Z7 /ZI /Zo /FdMozillaVPN.PDB /DEBUG
        QMAKE_LFLAGS_WINDOWS += /DEBUG
    }
    CONFIG += embed_manifest_exe
    DEFINES += MVPN_WINDOWS
    DEFINES += WIN32_LEAN_AND_MEAN
    # To avoid conficts between the 2 projects
    OBJECTS_DIR = .npobj
    MOC_DIR = .npmoc
} else:macos {
    CONFIG -= app_bundle
} else:!macos {
    error(Unsupported platform)
}

coverage {
    QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
    QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
}
