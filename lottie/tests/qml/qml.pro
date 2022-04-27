# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += qml quick

TEMPLATE = app
TARGET = tst_lottie

CONFIG += warn_on qmltestcase

macos {
    CONFIG -= app_bundle
}

HEADERS += helper.h
SOURCES += main.cpp

INCLUDEPATH += ../../lib

include(../../lottie.pri)

RESOURCES += qml.qrc

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
