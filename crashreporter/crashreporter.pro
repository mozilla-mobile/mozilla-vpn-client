# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += network
QT += quick
QT += widgets
QT += charts

CONFIG += c++1z
TEMPLATE = app

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../nebula/debug -lnebula
    LIBS += -L$$PWD/../glean/debug -lglean
}
CONFIG(release, debug|release) {
    LIBS += -L$$PWD/../nebula/release -lnebula
    LIBS += -L$$PWD/../glean/release -lglean
}

win*{
TARGET = MozillaVPN


QMAKE_CXXFLAGS += -MP -Zc:preprocessor

#Crashpad support
exists($$PWD/../3rdparty/crashpad/win64/release/include) {
    CONFIG(debug, debug|release) {
        LIBS += -L"$$PWD/../3rdparty/crashpad/win64/release/lib_md"
    }
    CONFIG(release, debug|release) {
        LIBS += -L"$$PWD/../3rdparty/crashpad/win64/release/lib_mt"
    }
    LIBS += -lutil
    LIBS += -lclient
    LIBS += -lbase
}else{
    error("Crashpad could not be found.  Have you run windows_compile.bat?")
}


CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += /Z7 /ZI /FdMozillaVPN.PDB /DEBUG
    QMAKE_LFLAGS_WINDOWS += /DEBUG
}

QT += networkauth
QT += svg
QT += winextras

}
