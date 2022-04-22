# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = lib
CONFIG += staticlib
TARGET = glean

QT += quick

HEADERS += glean.h
SOURCES += glean.cpp
RESOURCES += glean.qrc

!wasm {
    message(Include QSQlite plugin)
    QTPLUGIN += qsqlite
}

GLEAN_PINGS += pings.yaml

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc

## Perform glean generation
gleangen.input = GLEAN_PINGS
gleangen.output = $${_PRO_FILE_PWD_}/telemetry/telemetry.qrc
gleangen.commands = @echo Glean telemetry ${QMAKE_FILE_IN} \
    && cd $${_PRO_FILE_PWD_}/.. && scripts/utils/generate_glean.py
gleangen.depends += ${QMAKE_FILE_IN} ${QMAKE_FILE_IN_PATH}/metrics.yaml
gleangen.variable_out = RESOURCES
gleangen.clean_commands = rm -rf ${QMAKE_FILE_OUT_PATH}

## Dummy compiler for the glean samples header file
gleanhdr.input = GLEAN_PINGS
gleanhdr.output = $${_PRO_FILE_PWD_}/telemetry/gleansample.h
gleanhdr.commands = $$escape_expand(\\n)  # force creation of rule
gleanhdr.depends += $${_PRO_FILE_PWD_}/telemetry/telemetry.qrc
gleanhdr.variable_out = HEADERS

QMAKE_EXTRA_COMPILERS += gleangen gleanhdr
