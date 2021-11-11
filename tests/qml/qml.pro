# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
CONFIG += warn_on qmltestcase

DEFINES += APP_VERSION=\\\"QMLTest_AppVersion\\\"
DEFINES += BUILD_ID=\\\"QMLTest_BuildID\\\"

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00
DEFINES += UNIT_TEST

TEMPLATE = app
TARGET = qml_tests

OBJECTS_DIR = .obj
MOC_DIR = .moc

RESOURCES += \
    $$PWD/../../glean/glean.qrc \
    $$PWD/../../src/ui/compatQt5.qrc \
    $$PWD/../../src/ui/compatQt6.qrc \
    $$PWD/../../src/ui/components.qrc \
    $$PWD/../../src/ui/resources.qrc \
    $$PWD/../../src/ui/themes.qrc \
    $$PWD/../../src/ui/ui.qrc \

INCLUDEPATH += \
            . \
            ../../src \
            ../../translations/generated

HEADERS += \
    ../../src/mozillavpn.h \

SOURCES += \
    mocmozillavpn.cpp \
    main.cpp \
    ../../src/l18nstringsimpl.cpp \

exists($$PWD/../../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}
