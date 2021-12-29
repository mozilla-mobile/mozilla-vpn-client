# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/nebula.cpp

HEADERS += \
    $$PWD/nebula.h

RESOURCES += $$PWD/ui/components.qrc
RESOURCES += $$PWD/ui/themes.qrc
RESOURCES += $$PWD/ui/nebula_resources.qrc

QML_IMPORT_PATH += $$PWD/ui
QML_DESIGNER_IMPORT_PATH += $$PWD/ui

versionAtLeast(QT_VERSION, 6.0.0) {
    RESOURCES += $$PWD/ui/compatQt6.qrc
    RESOURCES += $$PWD/ui/resourcesQt6.qrc
} else {
    RESOURCES += $$PWD/ui/compatQt5.qrc
}
