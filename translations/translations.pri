# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This file contains anything related to the translations and the strings generation

exists($$PWD/generated/translations.qrc) {
    RESOURCES += $$PWD/generated/translations.qrc
} else {
    error("No translations.qrc! Have you imported the languages with ./scripts/utils/import_languages.py?")
}

INCLUDEPATH += $$PWD/generated
RESOURCES += $$PWD/languages.qrc
RESOURCES += $$PWD/servers.qrc
SOURCES += $$PWD/i18nstrings.cpp

STRING_SOURCES = $$PWD/strings.yaml

## This is necessary to ensure MOC picks up this class.
HEADERS += $$PWD/generated/i18nstrings.h

## Perform string generation
stringgen.input = STRING_SOURCES
stringgen.output = $$PWD/generated/i18nstrings.h
stringgen.commands = @echo Generating strings from ${QMAKE_FILE_IN} \
    && python3 $$PWD/../scripts/utils/generate_strings.py \
        -o ${QMAKE_FILE_OUT_PATH} ${QMAKE_FILE_IN}
stringgen.depends += ${QMAKE_FILE_IN}
stringgen.variable_out = HEADERS

## Dummy rule for the private source file.
stringsrc.input = STRING_SOURCES
stringsrc.output = $$PWD/generated/i18nstrings_p.cpp
stringsrc.commands = $$escape_expand(\\n)  # force creation of rule
stringsrc.depends += $$PWD/generated/i18nstrings.h
stringsrc.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += stringgen stringsrc
