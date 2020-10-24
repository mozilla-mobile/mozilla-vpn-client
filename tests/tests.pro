# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
TEMPLATE = app
TARGET = tests
INCLUDEPATH += .

HEADERS += \
    ../src/settingsholder.h \
    ../src/user.h \
    testuser.h

SOURCES += \
    ../src/user.cpp \
    mocsettingsholder.cpp \
    testuser.cpp
