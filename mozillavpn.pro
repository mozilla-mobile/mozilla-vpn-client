# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

!versionAtLeast(QT_VERSION, 5.14.0) {
    message("Cannot use Qt $${QT_VERSION}")
    
    !android {
        error("Use Qt 5.14 or newer")
    }
}

TEMPLATE = subdirs

SUBDIRS += glean
SUBDIRS += lottie
SUBDIRS += nebula
SUBDIRS += translations
SUBDIRS += src
SUBDIRS += tests/unit

src.depends = glean lottie nebula translations
tests/unit.depends = glean nebula translations

# separate flag because an extra dependency is needed: liboath
AUTHTEST {
    SUBDIRS += tests/auth
    tests/auth.depends = glean translations
}

QMLTEST {
    SUBDIRS += tests/qml
    tests/qml.depends = glean lottie nebula translations
}

linux:!android|macos|win* {
    SUBDIRS += tests/nativemessaging
}
