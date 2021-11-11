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
    $$PWD/../../src/ui/compatQt5.qrc \
    $$PWD/../../src/ui/compatQt6.qrc \
    $$PWD/../../src/ui/components.qrc \
    $$PWD/../../src/ui/resources.qrc \
    $$PWD/../../src/ui/themes.qrc \
    $$PWD/../../src/ui/ui.qrc \

INCLUDEPATH += \
            . \
            ../../src \
            ../../src/hacl-star \
            ../../src/hacl-star/kremlin \
            ../../src/hacl-star/kremlin/minimal \
            ../../translations/generated

HEADERS += \
    ../../src/constants.h \
    ../../src/errorhandler.h \
    ../../src/featurelist.h \
    ../../src/hawkauth.h \
    ../../src/hkdf.h \
    ../../src/inspector/inspectorwebsocketconnection.h \
    ../../src/ipaddress.h \
    ../../src/ipaddressrange.h \
    ../../src/l18nstrings.h \
    ../../src/leakdetector.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/feature.h \
    ../../src/mozillavpn.h \
    ../../src/networkmanager.h \
    ../../src/networkrequest.h \
    ../../src/rfc/rfc1918.h \
    ../../src/rfc/rfc4193.h \
    ../../src/rfc/rfc4291.h \
    ../../src/rfc/rfc5735.h \
    ../../src/settingsholder.h \
    ../../src/simplenetworkmanager.h \
    ../../src/task.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \
    ../../src/urlopener.h \

SOURCES += \
    mocmozillavpn.cpp \
    main.cpp \
    ../unit/mocinspectorwebsocketconnection.cpp \
    ../../src/constants.cpp \
    ../../src/errorhandler.cpp \
    ../../src/featurelist.cpp \
    ../../src/hawkauth.cpp \
    ../../src/hkdf.cpp \
    ../../src/ipaddress.cpp \
    ../../src/ipaddressrange.cpp \
    ../../src/l18nstringsimpl.cpp \
    ../../src/leakdetector.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/feature.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkrequest.cpp \
    ../../src/rfc/rfc1918.cpp \
    ../../src/rfc/rfc4193.cpp \
    ../../src/rfc/rfc4291.cpp \
    ../../src/rfc/rfc5735.cpp \
    ../../src/settingsholder.cpp \
    ../../src/simplenetworkmanager.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \
    ../../src/urlopener.cpp \

exists($$PWD/../../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}
