# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += network
QT += quick
QT += widgets

CONFIG += c++1z
# include($$PWD/../../nebula/nebula.pri)
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
    $$PWD/crashui.cpp \
    $$PWD/crashuploader.cpp

HEADERS += \
    $$PWD/crashclient.h \
    $$PWD/crashreporter.h \
    $$PWD/crashreporterapp.h \
    $$PWD/crashreporterfactory.h \
    $$PWD/crashserverclient.h \
    $$PWD/crashserverclientfactory.h \
    $$PWD/crashui.h \
    $$PWD/crashuploader.h

win*{
    SOURCES += \
        $$PWD/platforms/windows/wincrashreporter.cpp \
        $$PWD/platforms/windows/windowscrashclient.cpp
    HEADERS += \
        $$PWD/platforms/windows/wincrashreporter.h \
        $$PWD/platforms/windows/windowscrashclient.h

    QT += networkauth
    QT += svg
    QT += winextras
}

RESOURCES += \
    $$PWD/crash_resources.qrc \
    $$PWD/crashui.qrc



