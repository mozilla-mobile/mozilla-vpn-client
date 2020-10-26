# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
TEMPLATE = app
TARGET = testModels

INCLUDEPATH += .
INCLUDEPATH += ../src

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

# Platform-specific: MacOS
macos {
    # For the loginitem
    LIBS += -framework ServiceManagement
    LIBS += -framework Security

    OBJECTIVE_SOURCES += \
            ../src/platforms/macos/macosutils.mm

    OBJECTIVE_HEADERS += \
            ../src/platforms/macos/macosutils.h
}

# Platform-specific: IOS
else:ios {
    DEFINES += IOS_INTEGRATION

    OBJECTIVE_SOURCES += \
            ../src/platforms/ios/iosutils.mm

    OBJECTIVE_HEADERS += \
            ../src/platforms/ios/iosutils.h
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
