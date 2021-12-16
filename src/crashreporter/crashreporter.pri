# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += network
QT += quick
QT += widgets
QT += charts

CONFIG += c++1z

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/crashclient.cpp \
    $$PWD/crashreporter.cpp \
    $$PWD/crashreporterapp.cpp \
    $$PWD/crashreporterfactory.cpp \
    $$PWD/crashserverclient.cpp \
    $$PWD/crashserverclientfactory.cpp \
    $$PWD/crashui.cpp

HEADERS += \
    $$PWD/crashclient.h \
    $$PWD/crashconstants.h \
    $$PWD/crashconstants.h \
    $$PWD/crashdata.h \
    $$PWD/crashreporter.h \
    $$PWD/crashreporterapp.h \
    $$PWD/crashreporterfactory.h \
    $$PWD/crashserverclient.h \
    $$PWD/crashserverclientfactory.h \
    $$PWD/crashui.h


win*{
DEFINES+=NOMINMAX
SOURCES += \
    $$PWD/platforms/windows/wincrashreporter.cpp \
    $$PWD/platforms/windows/winserverdelegate.cpp \
    $$PWD/platforms/windows/wincrashdata.cpp \
        $$PWD/platforms/windows/windowscrashserverclient.cpp
HEADERS += \
    $$PWD/platforms/windows/wincrashreporter.h \
    $$PWD/platforms/windows/winserverdelegate.h \
    $$PWD/platforms/windows/wincrashdata.h \
    $$PWD/platforms/windows/windowscrashserverclient.h

QMAKE_CXXFLAGS += -Zc:preprocessor

#Crashpad support
exists($$PWD/../../3rdparty/crashpad/crashpad/package.h) {
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/third_party/mini_chromium/mini_chromium/base" -lbase
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/client" -lcommon
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/client" -lclient
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/handler" -lhandler
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/util" -lutil
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/snapshot" -lsnapshot
    LIBS += -L"$$PWD/../../3rdparty/crashpad/crashpad/out/Release/obj/snapshot" -lcontext
    INCLUDEPATH +="$$PWD/../../3rdparty/crashpad/crashpad/out/Release/gen"
    INCLUDEPATH +="$$PWD/../../3rdparty/crashpad/crashpad"
    INCLUDEPATH +="$$PWD/../../3rdparty/crashpad/crashpad/third_party/mini_chromium/mini_chromium"
    LIBS+=-ladvapi32
    LIBS+=-lPowrProf
    LIBS+=-lVersion
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

RESOURCES += \
    $$PWD/crashui.qrc



