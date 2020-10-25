# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
TEMPLATE = app
TARGET = testModels
INCLUDEPATH += .

HEADERS += \
    ../src/device.h \
    ../src/devicemodel.h \
    ../src/keys.h \
    ../src/logger.h \
    ../src/loghandler.h \
    ../src/server.h \
    ../src/servercity.h \
    ../src/servercountry.h \
    ../src/servercountrymodel.h \
    ../src/serverdata.h \
    ../src/settingsholder.h \
    ../src/user.h \
    testmodels.h

SOURCES += \
    ../src/device.cpp \
    ../src/devicemodel.cpp \
    ../src/keys.cpp \
    ../src/logger.cpp \
    ../src/loghandler.cpp \
    ../src/server.cpp \
    ../src/servercity.cpp \
    ../src/servercountry.cpp \
    ../src/servercountrymodel.cpp \
    ../src/serverdata.cpp \
    ../src/user.cpp \
    mocsettingsholder.cpp \
    testmodels.cpp

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
