# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += charts
QT += network

DEFINES += APP_VERSION=\\\"1234\\\"
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += UNIT_TEST

TEMPLATE = app
TARGET = tests

INCLUDEPATH += \
            . \
            ../src \
            ../src/hacl-star \
            ../src/hacl-star/kremlin \
            ../src/hacl-star/kremlin/minimal

HEADERS += \
    ../src/connectiondataholder.h \
    ../src/curve25519.h \
    ../src/errorhandler.h \
    ../src/localizer.h \
    ../src/logger.h \
    ../src/loghandler.h \
    ../src/models/device.h \
    ../src/models/devicemodel.h \
    ../src/models/keys.h \
    ../src/models/server.h \
    ../src/models/servercity.h \
    ../src/models/servercountry.h \
    ../src/models/servercountrymodel.h \
    ../src/models/serverdata.h \
    ../src/models/user.h \
    ../src/mozillavpn.h \
    ../src/networkrequest.h \
    ../src/releasemonitor.h \
    ../src/settingsholder.h \
    ../src/task.h \
    ../src/tasks/accountandservers/taskaccountandservers.h \
    ../src/tasks/adddevice/taskadddevice.h \
    ../src/tasks/function/taskfunction.h \
    ../src/timersingleshot.h \
    helper.h \
    testconnectiondataholder.h \
    testlocalizer.h \
    testlogger.h \
    testmodels.h \
    testreleasemonitor.h \
    testtasks.h

SOURCES += \
    ../src/connectiondataholder.cpp \
    ../src/curve25519.cpp \
    ../src/errorhandler.cpp \
    ../src/hacl-star/Hacl_Chacha20.c \
    ../src/hacl-star/Hacl_Chacha20Poly1305_32.c \
    ../src/hacl-star/Hacl_Curve25519_51.c \
    ../src/hacl-star/Hacl_Poly1305_32.c \
    ../src/localizer.cpp \
    ../src/logger.cpp \
    ../src/loghandler.cpp \
    ../src/models/device.cpp \
    ../src/models/devicemodel.cpp \
    ../src/models/keys.cpp \
    ../src/models/server.cpp \
    ../src/models/servercity.cpp \
    ../src/models/servercountry.cpp \
    ../src/models/servercountrymodel.cpp \
    ../src/models/serverdata.cpp \
    ../src/models/user.cpp \
    ../src/releasemonitor.cpp \
    ../src/tasks/accountandservers/taskaccountandservers.cpp \
    ../src/tasks/adddevice/taskadddevice.cpp \
    ../src/timersingleshot.cpp \
    main.cpp \
    moccontroller.cpp \
    mocmozillavpn.cpp \
    mocnetworkrequest.cpp \
    mocsettingsholder.cpp \
    testconnectiondataholder.cpp \
    testlocalizer.cpp \
    testlogger.cpp \
    testmodels.cpp \
    testreleasemonitor.cpp \
    testtasks.cpp

# Platform-specific: Linux
linux {
    # QMAKE_CXXFLAGS *= -Werror
}

# Platform-specific: MacOS
else:macos {
    # For the loginitem
    LIBS += -framework ServiceManagement
    LIBS += -framework Security

    # QMAKE_CXXFLAGS *= -Werror

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

equals(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
    QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
}
