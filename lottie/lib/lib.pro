# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = lib
CONFIG += staticlib
TARGET = lottie

QT += quick

SOURCES += lottie.cpp \
           lottieprivate.cpp \
           lottieprivatedocument.cpp \
           lottieprivatenavigator.cpp \
           lottieprivatewindow.cpp

HEADERS += lottie.h \
           lottieprivate.h \
           lottieprivatedocument.h \
           lottieprivatenavigator.h \
           lottieprivatewindow.h \
           lottiestatus.h

# wrap lottie in a single file before using it as resource.
#
# TODO: Can we make this a bit more location-agnostic???
# It's kind of important that this rule happens, but custom
# Compilers are broken on MSVC and XCode genrators.
LOTTIEWRAP = $$cat(lottie/lottie_wrap.js.template, blob)
LOTTIEJS = $$cat(lottie/lottie.min.js, blob)
LOTTIERC = $$replace(LOTTIEWRAP, __LOTTIE__, $$LOTTIEJS)
write_file(lottie/lottie.mjs, LOTTIERC)

RESOURCES += lottie.qrc

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
