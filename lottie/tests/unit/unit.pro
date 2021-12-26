# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = app
TARGET = tests

INCLUDEPATH += ../../lib

HEADERS += \
    ../../lib/lottieprivate.h \
    ../../lib/lottieprivatedocument.h \
    ../../lib/lottieprivatenavigator.h \
    ../../lib/lottieprivatewindow.h \
    helper.h \
    testdocument.h \
    testnavigator.h \
    testwindow.h

SOURCES += \
    ../../lib/lottieprivate.cpp \
    ../../lib/lottieprivatedocument.cpp \
    ../../lib/lottieprivatenavigator.cpp \
    ../../lib/lottieprivatewindow.cpp \
    main.cpp \
    testdocument.cpp \
    testnavigator.cpp \
    testwindow.cpp

CONFIG += debug

QT += qml quick testlib

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
