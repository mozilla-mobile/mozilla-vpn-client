# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

CONFIG += c++1z

TEMPLATE  = app
TARGET = mozillavpnnp

# No QT for this project
QT =

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
        vpnconnection.h

linux:!android {
    target.path = /usr/lib/mozillavpn
    INSTALLS += target

    manifest.path = /usr/lib/mozilla/native-messaging-hosts
    manifest.files = manifests/linux/mozillavpn.json
    INSTALLS += manifest
} else:win* {
    CONFIG += embed_manifest_exe
    DEFINES += MVPN_WINDOWS

    # To avoid conficts between the 2 projects
    OBJECTS_DIR = .npobj
    MOC_DIR = .npmoc
} else {
    error(Unsupported platform)
}

equals(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
    QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
}
