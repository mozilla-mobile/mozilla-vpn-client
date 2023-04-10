# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

exists($$PWD/telemetry/telemetry.qrc) {
    RESOURCES += $$PWD/telemetry/telemetry.qrc
}
else{
    error(Glean generated files are missing. Please run `python3 ./scripts/utils/generate_glean.py`)
}

INCLUDEPATH += $$PWD
SOURCES += $$PWD/glean.cpp
HEADERS += $$PWD/glean.h

!wasm {
    message(Include QSQlite plugin)
    QTPLUGIN += qsqlite
}

PYTHON_BIN = $$system(which python3)
isEmpty(PYTHON_BIN) {
    PYTHON_BIN = python3
}
message(Using python: $$PYTHON_BIN)

gleandefs.input = GLEAN_METRICS
gleandefs.output = $$PWD/telemetry/gleansample.h
gleandefs.commands = @echo Generating gleansample.h from ${QMAKE_FILE_IN} \
    && $$PYTHON_BIN $$PWD/../scripts/utils/generate_gleandefs.py \
        -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN} -f cpp
gleandefs.CONFIG = combine
gleandefs.variable_out = HEADERS

GLEAN_METRICS = $$PWD/../src/apps/vpn/telemetry/metrics_deprecated.yaml \
                $$PWD/../src/shared/telemetry/metrics_deprecated.yaml
QMAKE_EXTRA_COMPILERS += gleandefs

QML_IMPORT_PATH += $$PWD
RESOURCES += $$PWD/glean.qrc
