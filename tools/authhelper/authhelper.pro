# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += network
QT += networkauth
QT += qml
QT += widgets

TEMPLATE = app
TARGET = authhelper

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION=\\\"0.1\\\"
DEFINES += BUILD_ID=\\\"1234\\\"

DEFINES += MVPN_DEBUG
DEFINES += MVPN_DUMMY
DEFINES += UNIT_TEST

CONFIG += c++1z

macos {
    CONFIG -= app_bundle
}

win* {
    QMAKE_CXXFLAGS += -MP -Zc:preprocessor
}

CONFIG += link_pkgconfig
PKGCONFIG += liboath

INCLUDEPATH += \
            . \
            ../../src \
            ../../src/hacl-star \
            ../../src/hacl-star/kremlin \
            ../../src/hacl-star/kremlin/minimal \
            ../../translations/generated

HEADERS += \
    ../../src/authenticationinapp/authenticationinapp.h \
    ../../src/authenticationinapp/authenticationinapplistener.h \
    ../../src/authenticationinapp/incrementaldecoder.h \
    ../../src/authenticationlistener.h \
    ../../src/constants.h \
    ../../src/errorhandler.h \
    ../../src/featurelist.h \
    ../../src/hawkauth.h \
    ../../src/hkdf.h \
    ../../src/inspector/inspectorhandler.h \
    ../../src/leakdetector.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/feature.h \
    ../../src/mozillavpn.h \
    ../../src/networkmanager.h \
    ../../src/networkrequest.h \
    ../../src/settingsholder.h \
    ../../src/simplenetworkmanager.h \
    ../../src/task.h \
    ../../src/tasks/authenticate/desktopauthenticationlistener.h \
    ../../src/tasks/authenticate/taskauthenticate.h \
    ../../src/tasks/function/taskfunction.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \
    ../../src/urlopener.h \
    ../../tests/auth/utils.h \


SOURCES += \
    ../../src/authenticationinapp/authenticationinapp.cpp \
    ../../src/authenticationinapp/authenticationinapplistener.cpp \
    ../../src/authenticationinapp/incrementaldecoder.cpp \
    ../../src/authenticationlistener.cpp \
    ../../src/constants.cpp \
    ../../src/errorhandler.cpp \
    ../../src/featurelist.cpp \
    ../../src/hawkauth.cpp \
    ../../src/hkdf.cpp \
    ../../src/l18nstringsimpl.cpp \
    ../../src/leakdetector.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/feature.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkrequest.cpp \
    ../../src/settingsholder.cpp \
    ../../src/simplenetworkmanager.cpp \
    ../../src/tasks/authenticate/desktopauthenticationlistener.cpp \
    ../../src/tasks/authenticate/taskauthenticate.cpp \
    ../../src/tasks/function/taskfunction.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \
    ../../src/urlopener.cpp \
    ../../tests/auth/mocmozillavpn.cpp \
    ../../tests/auth/utils.cpp \
    ../../tests/unit/mocinspectorhandler.cpp \
    main.cpp

exists($$PWD/../../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
