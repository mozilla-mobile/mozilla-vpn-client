# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

exists($$PWD/telemetry/gleansample.h) {
    HEADERS += $$PWD/telemetry/gleansample.h
}
else{
    error(Glean generated files are missing. Please run `python3 ./scripts/utils/generate_glean.py`)
}

SOURCES += \
    $$PWD/glean.cpp

HEADERS += \
    $$PWD/glean.h

!wasm {
    message(Include QSQlite plugin)
    QTPLUGIN += qsqlite
}

QML_IMPORT_PATH += $$PWD
RESOURCES += $$PWD/glean.qrc
