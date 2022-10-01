# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += network
QT += networkauth
QT += qml
QT += widgets
QT += websockets

CONFIG += c++1z

macos {
    CONFIG -= app_bundle
}

DEFINES += BUILD_QMAKE

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00
DEFINES += UNIT_TEST
DEFINES += MVPN_DEBUG
DEFINES += MVPN_DUMMY

TEMPLATE = app
TARGET = tests

RESOURCES += auth.qrc

INCLUDEPATH += \
            . \
            ../../src \
            ../../src/hacl-star \
            ../../src/hacl-star/kremlin \
            ../../src/hacl-star/kremlin/minimal

HEADERS += \
    ../../src/authenticationinapp/authenticationinapp.h \
    ../../src/authenticationinapp/authenticationinapplistener.h \
    ../../src/authenticationinapp/authenticationinappsession.h \
    ../../src/authenticationinapp/incrementaldecoder.h \
    ../../src/authenticationlistener.h \
    ../../src/constants.h \
    ../../src/env.h \
    ../../src/errorhandler.h \
    ../../src/hawkauth.h \
    ../../src/hkdf.h \
    ../../src/inspector/inspectorhandler.h \
    ../../src/ipaddress.h \
    ../../src/leakdetector.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/feature.h \
    ../../src/models/featuremodel.h \
    ../../src/models/server.h \
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
    ../../src/tasks/authenticate/desktopauthenticationlistener.h \
    ../../src/tasks/authenticate/taskauthenticate.h \
    ../../src/tasks/deleteaccount/taskdeleteaccount.h \
    ../../src/tasks/function/taskfunction.h \
    ../../src/taskscheduler.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \
    ../../src/update/webupdater.h \
    ../../src/urlopener.h \
    testemailvalidation.h \
    testpasswordvalidation.h \
    testsignupandin.h

SOURCES += \
    mocmozillavpn.cpp \
    ../unit/mocinspectorhandler.cpp \
    ../../src/authenticationinapp/authenticationinapp.cpp \
    ../../src/authenticationinapp/authenticationinapplistener.cpp \
    ../../src/authenticationinapp/authenticationinappsession.cpp \
    ../../src/authenticationinapp/incrementaldecoder.cpp \
    ../../src/authenticationlistener.cpp \
    ../../src/constants.cpp \
    ../../src/errorhandler.cpp \
    ../../src/hawkauth.cpp \
    ../../src/hkdf.cpp \
    ../../src/ipaddress.cpp \
    ../../src/leakdetector.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/feature.cpp \
    ../../src/models/featuremodel.cpp \
    ../../src/models/server.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkrequest.cpp \
    ../../src/rfc/rfc1918.cpp \
    ../../src/rfc/rfc4193.cpp \
    ../../src/rfc/rfc4291.cpp \
    ../../src/rfc/rfc5735.cpp \
    ../../src/settingsholder.cpp \
    ../../src/simplenetworkmanager.cpp \
    ../../src/tasks/authenticate/desktopauthenticationlistener.cpp \
    ../../src/tasks/authenticate/taskauthenticate.cpp \
    ../../src/tasks/deleteaccount/taskdeleteaccount.cpp \
    ../../src/tasks/function/taskfunction.cpp \
    ../../src/taskscheduler.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \
    ../../src/update/webupdater.cpp \
    ../../src/urlopener.cpp \
    main.cpp \
    testemailvalidation.cpp \
    testpasswordvalidation.cpp \
    testsignupandin.cpp

include($$PWD/../../version.pri)
include($$PWD/../../glean/glean.pri)
include($$PWD/../../translations/translations.pri)

win* {
    QMAKE_CXXFLAGS += -MP -Zc:preprocessor
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
