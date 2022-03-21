# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This file contains anything related to the translations and the strings generation

# TODO: move the string generation here.
exists($$PWD/../../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}

# TODO: move the language import here
exists($$PWD/../../translations/translations.pri) {
    include($$PWD/../../translations/translations.pri)
} else {
    message(Languages were not imported - using fallback English)
    TRANSLATIONS += \
        $$PWD/../../translations/en/mozillavpn_en.ts

    ts.commands += lupdate $$PWD -no-obsolete -ts $$PWD/../../translations/en/mozillavpn_en.ts
    ts.CONFIG += no_check_exist
    ts.output = $$PWD/../../translations/en/mozillavpn_en.ts
    ts.input = $$PWD/..
    QMAKE_EXTRA_TARGETS += ts
    PRE_TARGETDEPS += ts
}

QMAKE_LRELEASE_FLAGS += -idbased
CONFIG += lrelease
CONFIG += embed_translations

RESOURCES += $$PWD/../../translations/servers.qrc
