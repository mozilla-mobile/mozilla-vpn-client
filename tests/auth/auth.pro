# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += network
QT += networkauth
QT += qml
QT += widgets

DEFINES += APP_VERSION=\\\"1234\\\"
DEFINES += BUILD_ID=\\\"1234\\\"

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += UNIT_TEST

config += debug

TEMPLATE = app
TARGET = tests

CONFIG += link_pkgconfig
PKGCONFIG += liboath

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
    ../../src/authenticationinapp/incrementaldecoder.h \
    ../../src/authenticationlistener.h \
    ../../src/constants.h \
    ../../src/errorhandler.h \
    ../../src/featurelist.h \
    ../../src/hawkauth.h \
    ../../src/hkdf.h \
    ../../src/inspector/inspectorwebsocketconnection.h \
    ../../src/ipaddress.h \
    ../../src/ipaddressrange.h \
    ../../src/leakdetector.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
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
    ../../src/urlopener.h \
    ../../translations/generated/l18nstrings.h \
    testemailvalidation.h \
    testpasswordvalidation.h \
    testsignupandin.h

SOURCES += \
    mocmozillavpn.cpp \
    ../unit/mocinspectorwebsocketconnection.cpp \
    ../../src/authenticationinapp/authenticationinapp.cpp \
    ../../src/authenticationinapp/authenticationinapplistener.cpp \
    ../../src/authenticationinapp/incrementaldecoder.cpp \
    ../../src/authenticationlistener.cpp \
    ../../src/constants.cpp \
    ../../src/errorhandler.cpp \
    ../../src/featurelist.cpp \
    ../../src/hawkauth.cpp \
    ../../src/hkdf.cpp \
    ../../src/ipaddress.cpp \
    ../../src/ipaddressrange.cpp \
    ../../src/leakdetector.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
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
    ../../src/urlopener.cpp \
    main.cpp \
    testemailvalidation.cpp \
    testpasswordvalidation.cpp \
    testsignupandin.cpp

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

coverage {
    QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
    QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
}
