# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: dummy)

CONFIG += c++1z

win* {
  CONFIG += embed_manifest_exe
} else {
  QMAKE_CXXFLAGS *= -Werror
}

TARGET = mozillavpn

DEFINES += MVPN_DUMMY

SOURCES += \
        platforms/dummy/dummycontroller.cpp \
        platforms/dummy/dummycryptosettings.cpp \
        systemtraynotificationhandler.cpp \
        tasks/authenticate/desktopauthenticationlistener.cpp

HEADERS += \
        platforms/dummy/dummycontroller.h \
        systemtraynotificationhandler.h \
        tasks/authenticate/desktopauthenticationlistener.h
