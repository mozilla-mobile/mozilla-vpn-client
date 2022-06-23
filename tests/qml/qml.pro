# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = app
TARGET = qml_tests

QT += quick
QT += widgets
QT += websockets

CONFIG += warn_on qmltestcase
CONFIG += c++1z

macos {
    CONFIG -= app_bundle
}

# Allows us to be a supported platform
DEFINES += UNIT_TEST
DEFINES += MVPN_DUMMY

# Sets up app and build id which we test for in test_VPNAboutUs
DEFINES += BUILD_QMAKE

RESOURCES += \
    $$PWD/../../src/ui/ui.qrc \

INCLUDEPATH += \
            . \
            ../../src \
            ../../lottie/lib

include($$PWD/../../glean/glean.pri)
include($$PWD/../../lottie/lottie.pri)
include($$PWD/../../nebula/nebula.pri)
include($$PWD/../../translations/translations.pri)

SOURCES += \
    helper.cpp \
    main.cpp \
    mocconstants.cpp \
    mocmozillavpn.cpp \
    ../unit/mocinspectorhandler.cpp \
    ../../src/closeeventhandler.cpp \
    ../../src/hawkauth.cpp \
    ../../src/hkdf.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/feature.cpp \
    ../../src/models/featuremodel.cpp \
    ../../src/models/server.cpp \
    ../../src/models/whatsnewmodel.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkrequest.cpp \
    ../../src/settingsholder.cpp \
    ../../src/theme.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \
    ../../src/update/webupdater.cpp

HEADERS += \
    helper.h \
    ../../src/closeeventhandler.h \
    ../../src/constants.h \
    ../../src/hawkauth.h \
    ../../src/hkdf.h \
    ../../src/inspector/inspectorhandler.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/feature.h \
    ../../src/models/featuremodel.h \
    ../../src/models/whatsnewmodel.h \
    ../../src/mozillavpn.h \
    ../../src/networkmanager.h \
    ../../src/networkrequest.h \
    ../../src/settingsholder.h \
    ../../src/theme.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \
    ../../src/update/webupdater.h

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc

win* {
    QMAKE_CXXFLAGS += -MP -Zc:preprocessor

    versionAtLeast(QT_VERSION, 6.0.0) {
        versionAtLeast(QT_VERSION, 6.3.0) {
            # See https://mozilla-hub.atlassian.net/browse/VPN-1894
	    error(Remove the qt6 windows hack!)
        }
        RESOURCES += qt6winhack.qrc
    }
}
