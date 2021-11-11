# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
CONFIG += warn_on qmltestcase

TEMPLATE = app
TARGET = qml_tests

SOURCES += tst_main.cpp

OBJECTS_DIR = .obj
MOC_DIR = .moc

RESOURCES += \
    $$PWD/../../src/ui/components.qrc \
    $$PWD/../../src/ui/resources.qrc \
    $$PWD/../../src/ui/themes.qrc \
    $$PWD/../../src/ui/ui.qrc \