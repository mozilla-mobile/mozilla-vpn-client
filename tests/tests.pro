# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
TEMPLATE = app
TARGET = tests
INCLUDEPATH += .

HEADERS += \
    ../src/logger.h \
    ../src/loghandler.h \
    ../src/server.h \
    ../src/servercity.h \
    ../src/servercountry.h \
    ../src/serverdata.h \
    ../src/settingsholder.h \
    ../src/user.h \
    autotest.h \
    testserverdata.h \
    testuser.h

SOURCES += \
    ../src/logger.cpp \
    ../src/loghandler.cpp \
    ../src/server.cpp \
    ../src/servercity.cpp \
    ../src/servercountry.cpp \
    ../src/serverdata.cpp \
    ../src/user.cpp \
    main.cpp \
    mocsettingsholder.cpp \
    testserverdata.cpp \
    testuser.cpp
