# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += network

TEMPLATE = app
TARGET = ipmonitor

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION=\\\"0.1\\\"

macos {
    DEFINES += MVPN_MACOS
    CONFIG -= app_bundle
}
else:linux:!android {
    DEFINES += MVPN_LINUX
}

HEADERS += \
        ../../src/ipaddress.h \
        ../../src/leakdetector.h \
        ../../src/loghandler.h \
        ../../src/logger.h \
        ../../src/rfc/rfc1918.h
SOURCES += \
        main.cpp \
        ../../src/ipaddress.cpp \
        ../../src/leakdetector.cpp \
        ../../src/loghandler.cpp \
        ../../src/logger.cpp \
        ../../src/rfc/rfc1918.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libpcap

INCLUDEPATH += ../../src

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
