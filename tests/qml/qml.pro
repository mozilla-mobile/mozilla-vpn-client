# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
TEMPLATE = app
TARGET = qml_tests

QT += quick

CONFIG += warn_on qmltestcase

# Allows us to be a supported platform
DEFINES += UNIT_TEST
DEFINES += MVPN_DUMMY
# Sets up app and build id which we test for in test_VPNAboutUs
DEFINES += APP_VERSION=\\\"QMLTest_AppVersion\\\"
DEFINES += BUILD_ID=\\\"QMLTest_BuildID\\\"

RESOURCES += \
    $$PWD/../../src/ui/compatQt5.qrc \
    $$PWD/../../src/ui/compatQt6.qrc \
    $$PWD/../../src/ui/components.qrc \
    $$PWD/../../src/ui/resources.qrc \
    $$PWD/../../src/ui/themes.qrc \
    $$PWD/../../src/ui/ui.qrc \

INCLUDEPATH += \
            . \
            ../../src \
            ../glean/telemetry \
            ../../translations/generated \

SOURCES += \
    helper.cpp \
    main.cpp \
    mocmozillavpn.cpp \
    ../unit/mocinspectorwebsocketconnection.cpp \
    ../../src/closeeventhandler.cpp \
    ../../src/constants.cpp \
    ../../src/featurelist.cpp \
    ../../src/hawkauth.cpp \
    ../../src/hkdf.cpp \
    ../../src/l18nstringsimpl.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/feature.cpp \
    ../../src/models/whatsnewmodel.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkrequest.cpp \
    ../../src/settingsholder.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \

HEADERS += \
    helper.h \
    ../../src/closeeventhandler.h \
    ../../src/constants.h \
    ../../src/featurelist.h \
    ../../src/hawkauth.h \
    ../../src/hkdf.h \
    ../../src/inspector/inspectorwebsocketconnection.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/feature.h \
    ../../src/models/whatsnewmodel.h \
    ../../src/mozillavpn.h \
    ../../src/networkmanager.h \
    ../../src/networkrequest.h \
    ../../src/settingsholder.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \

exists($$PWD/../../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}

exists($$PWD/../../glean/telemetry/gleansample.h) {
    RESOURCES += $$PWD/../../glean/glean.qrc
} else {
    error(Glean generated files are missing. Please run `python3 ./scripts/generate_glean.py`)
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc