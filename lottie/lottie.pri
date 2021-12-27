# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += $$PWD/lib/lottie.cpp \
           $$PWD/lib/lottieprivate.cpp \
           $$PWD/lib/lottieprivatedocument.cpp \
           $$PWD/lib/lottieprivatenavigator.cpp \
           $$PWD/lib/lottieprivatewindow.cpp

HEADERS += $$PWD/lib/lottie.h \
           $$PWD/lib/lottieprivate.h \
           $$PWD/lib/lottieprivatedocument.h \
           $$PWD/lib/lottieprivatenavigator.h \
           $$PWD/lib/lottieprivatewindow.h \
           $$PWD/lib/lottiestatus.h

RESOURCES += $$PWD/lib/lottie.qrc

CONFIG += qmltypes

QML_IMPORT_NAME = vpn.mozilla.lottie
QML_IMPORT_MAJOR_VERSION = 1
