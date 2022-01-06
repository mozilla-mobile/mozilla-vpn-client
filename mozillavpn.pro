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
SUBDIRS += nebula
SUBDIRS += src
SUBDIRS += tests/unit
SUBDIRS += lottie/tests/unit

# separate flag because an extra dependency is needed: liboath
AUTHTEST {
    SUBDIRS += tests/auth
}

QMLTEST {
    SUBDIRS += tests/qml
    SUBDIRS += lottie/tests/qml
}

webextension {
    SUBDIRS += extension/app
    SUBDIRS += tests/nativemessaging
}

TOOLS {
    SUBDIRS += tools/ipmonitor
}
