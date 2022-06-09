# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: wasm)

DEFINES += MVPN_DUMMY
DEFINES += MVPN_WASM

QMAKE_CXXFLAGS *= -Werror

TARGET = mozillavpn

# sql not available for wasm.
QT -= sql
# Not available in wasm
QT -= networkauth

CONFIG += c++1z

# 32Mb
QMAKE_WASM_TOTAL_MEMORY=33554432
QMAKE_LFLAGS+= "-s TOTAL_MEMORY=33554432"

SOURCES += \
    platforms/dummy/dummycontroller.cpp \
    platforms/dummy/dummycryptosettings.cpp \
    platforms/macos/macosmenubar.cpp \
    platforms/wasm/wasmauthenticationlistener.cpp \
    platforms/wasm/wasmnetworkrequest.cpp \
    platforms/wasm/wasmnetworkwatcher.cpp \
    platforms/wasm/wasmwindowcontroller.cpp \
    platforms/wasm/wasminspector.cpp \
    systemtraynotificationhandler.cpp

HEADERS += \
    platforms/dummy/dummycontroller.h \
    platforms/macos/macosmenubar.h \
    platforms/wasm/wasmauthenticationlistener.h \
    platforms/wasm/wasmnetworkwatcher.h \
    platforms/wasm/wasmwindowcontroller.h \
    platforms/wasm/wasminspector.h \
    systemtraynotificationhandler.h

SOURCES -= networkrequest.cpp
RESOURCES += platforms/wasm/networkrequests.qrc
