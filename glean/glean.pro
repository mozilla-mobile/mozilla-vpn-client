# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT -= gui
QT += qml
TEMPLATE = lib

CONFIG += staticlib
CONFIG += c++1z

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

exists($$PWD/telemetry/gleansample.h) {
    HEADERS+=$$PWD/telemetry/gleansample.h
}
else{
    error(Glean generated files are missing. Please run `python3 ./scripts/generate_glean.py`)
}

SOURCES += \
    glean.cpp

HEADERS += \
    glean.h

!wasm {
    message(Include QSQlite plugin)
    QTPLUGIN += qsqlite
}

QML_IMPORT_PATH+=$$PWD
RESOURCES += glean.qrc
CONFIG(debug, debug|release) {
    DESTDIR=debug
}
CONFIG(release, debug|release) {
    DESTDIR=release
}
android {
    CONFIG(debug, debug|release) {
        DESTDIR=$$ANDROID_TARGET_ARCH/debug
    }
    CONFIG(release, debug|release) {
        DESTDIR=$$ANDROID_TARGET_ARCH/release
    }
    message(Output folder: $$DESTDIR)
}
