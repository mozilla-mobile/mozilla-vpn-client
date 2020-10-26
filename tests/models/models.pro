# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
TEMPLATE = app
TARGET = testModels

INCLUDEPATH += .
INCLUDEPATH += ../../src

HEADERS += \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/device.h \
    ../../src/models/devicemodel.h \
    ../../src/models/keys.h \
    ../../src/models/server.h \
    ../../src/models/servercity.h \
    ../../src/models/servercountry.h \
    ../../src/models/servercountrymodel.h \
    ../../src/models/serverdata.h \
    ../../src/models/user.h \
    ../../src/settingsholder.h \
    testmodels.h

SOURCES += \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/device.cpp \
    ../../src/models/devicemodel.cpp \
    ../../src/models/keys.cpp \
    ../../src/models/server.cpp \
    ../../src/models/servercity.cpp \
    ../../src/models/servercountry.cpp \
    ../../src/models/servercountrymodel.cpp \
    ../../src/models/serverdata.cpp \
    ../../src/models/user.cpp \
    mocsettingsholder.cpp \
    testmodels.cpp

# Platform-specific: MacOS
macos {
    # For the loginitem
    LIBS += -framework ServiceManagement
    LIBS += -framework Security

    OBJECTIVE_SOURCES += \
            ../../src/platforms/macos/macosutils.mm

    OBJECTIVE_HEADERS += \
            ../../src/platforms/macos/macosutils.h
}

# Platform-specific: IOS
else:ios {
    DEFINES += IOS_INTEGRATION

    OBJECTIVE_SOURCES += \
            ../../src/platforms/ios/iosutils.mm

    OBJECTIVE_HEADERS += \
            ../../src/platforms/ios/iosutils.h
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

linux-clang++ {
    QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
    QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
}
