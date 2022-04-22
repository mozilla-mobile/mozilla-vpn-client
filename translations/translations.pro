# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TEMPLATE = lib
CONFIG += staticlib
TARGET = translations

QT += quick

INCLUDEPATH += $$PWD/generated

HEADERS += generated/l18nstrings.h
SOURCES += l18nstrings.cpp
RESOURCES += servers.qrc

exists($$PWD/generated/translations.qrc) {
    RESOURCES += generated/translations.qrc
} else {
    error("No translations.qrc! Have you imported the languages?")
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc

STRING_SOURCES = strings.yaml

## Perform string generation
## TODO: This also depends vaguely on the contents of the guide and tutorials.
## TODO: I would like to make the guide and tutorial translations loaded dynamically.
stringgen.input = STRING_SOURCES
stringgen.output = generated/l18nstrings.h
stringgen.commands = @echo Generating strings from ${QMAKE_FILE_IN} \
    && python3 $${_PRO_FILE_PWD_}/../scripts/utils/generate_strings.py \
        -g ${QMAKE_FILE_IN_PATH}/../src/ui/guides \
        -t ${QMAKE_FILE_IN_PATH}/../src/ui/tutorials \
        -o ${QMAKE_FILE_OUT_PATH} ${QMAKE_FILE_IN}
stringgen.depends += ${QMAKE_FILE_IN}
stringgen.variable_out = HEADERS

## Dummy rule for the private source file.
stringsrc.input = STRING_SOURCES
stringsrc.output = generated/l18nstrings_p.cpp
stringsrc.commands = $$escape_expand(\\n)  # force creation of rule
stringsrc.depends += generated/l18nstrings.h
stringsrc.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += stringgen stringsrc
