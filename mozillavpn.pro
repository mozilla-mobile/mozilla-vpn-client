# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

!versionAtLeast(QT_VERSION, 5.15.0) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 5.15 or newer")
}

TEMPLATE = subdirs
SUBDIRS += src
SUBDIRS += tests/unit

linux {
    SUBDIRS += linux
}
