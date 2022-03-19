# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

!no_glean_gen {
    # This cannot be an EXTRA_TARGET because it would require PRE_TARGETDEPS
    # but this is not supported by xcode/vs

    glean.input = GLEAN
    glean.output = $$PWD/../${QMAKE_FILE_IN}
    glean.commands = python3 $$PWD/../../scripts/qmake/glean.py build
    glean.clean_commands = python3 $$PWD/../../scripts/qmake/glean.py clean
    glean.CONFIG = target_predeps no_link

    GLEAN += ../glean/telemetry/qmldir

    QMAKE_EXTRA_COMPILERS += glean
}

SOURCES += \
    $$PWD/../../glean/glean.cpp

HEADERS += \
    $$PWD/../../glean/glean.h

!wasm {
    message(Include QSQlite plugin)
    QTPLUGIN += qsqlite
}

QML_IMPORT_PATH += $$PWD/../../glean
RESOURCES += $$PWD/../../glean/glean.qrc
