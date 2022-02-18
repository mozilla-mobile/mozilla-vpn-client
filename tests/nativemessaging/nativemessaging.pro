# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += network

macos {
    CONFIG -= app_bundle
}

TEMPLATE = app
TARGET = tests

HEADERS += \
    helper.h \
    helperserver.h \
    testbridge.h \
    testnoop.h

SOURCES += \
    main.cpp \
    helper.cpp \
    helperserver.cpp \
    testbridge.cpp \
    testnoop.cpp

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
