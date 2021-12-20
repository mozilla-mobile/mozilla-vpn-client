# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = app
TARGET = rccmerge

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION=\\\"0.1\\\"

macos {
    CONFIG -= app_bundle
}

SOURCES += main.cpp \
           ../../src/resourceloaderutils.cpp

HEADERS += ../../src/resourceloaderutils.h

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
