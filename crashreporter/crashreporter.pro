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
        crashdata.cpp \
        crashreporter.cpp \
        crashreporterfactory.cpp \
        main.cpp \
        platforms/windows/wincrashdata.cpp


HEADERS += \
    crashconstants.h \
    crashdata.h \
    crashreporter.h \
    crashreporterfactory.h \
    platforms/windows/wincrashdata.h


#CONFIG(debug, debug|release) {
#    DEFINES += MVPN_DEBUG
#    LIBS += -L$$PWD/../nebula/debug -lnebula
#    LIBS += -L$$PWD/../glean/debug -lglean
#}
#CONFIG(release, debug|release) {
#    LIBS += -L$$PWD/../nebula/release -lnebula
#    LIBS += -L$$PWD/../glean/release -lglean
#}



win*{
SOURCES += \
    platforms/windows/wincrashreporter.cpp \
    platforms/windows/winserverdelegate.cpp
HEADERS += \
    platforms/windows/wincrashreporter.h \
    platforms/windows/winserverdelegate.h

QMAKE_CXXFLAGS += -Zc:preprocessor

#Crashpad support
exists($$PWD/../3rdparty/crashpad/crashpad/package.h) {
    LIBS += -L"$$PWD/../3rdparty/crashpad/crashpad/out/Release/obj/third_party/mini_chromium/mini_chromium/base" -lbase
    LIBS += -L"$$PWD/../3rdparty/crashpad/crashpad/out/Release/obj/client" -lcommon
    LIBS += -L"$$PWD/../3rdparty/crashpad/crashpad/out/Release/obj/handler" -lhandler
    LIBS += -L"$$PWD/../3rdparty/crashpad/crashpad/out/Release/obj/util" -lutil
    INCLUDEPATH +="$$PWD/../3rdparty/crashpad/crashpad"
    INCLUDEPATH +="$$PWD/../3rdparty/crashpad/crashpad/third_party/mini_chromium/mini_chromium"
    LIBS+=-ladvapi32
#    QMAKE_LFLAGS += /NODEFAULTLIB:libcmt.lib
#    QMAKE_LFLAGS += /NODEFAULTLIB:handler.lib
#    QMAKE_LFLAGS += /NODEFAULTLIB:common.lib

}else{
    error("Crashpad could not be found.  Have you run windows_compile.bat?")
}


#CONFIG(debug, debug|release) {
#    QMAKE_LFLAGS_WINDOWS += /DEBUG
#}

QT += networkauth
QT += svg
QT += winextras

}



