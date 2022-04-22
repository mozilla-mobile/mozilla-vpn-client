# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = lib
CONFIG += staticlib
TARGET = nebula

QT += quick

SOURCES += nebula.cpp
HEADERS += nebula.h

RESOURCES += ui/components.qrc
RESOURCES += ui/themes.qrc
RESOURCES += ui/nebula_resources.qrc

QML_IMPORT_PATH += ui

versionAtLeast(QT_VERSION, 6.0.0) {
    RESOURCES += ui/compatQt6.qrc
    RESOURCES += ui/resourcesQt6.qrc
} else {
    RESOURCES += ui/compatQt5.qrc
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
