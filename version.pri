# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

!defined(VERSION, var):VERSION = 2.9.0

VERSION_MAJOR = $$section(VERSION, ., 0, 0)
!defined(BUILD_ID, var) {
    win* {
        BUILD_ID = $${VERSION_MAJOR}.$$system(powershell Get-Date -Format yyyyMMddHHmm)
    } else {
        BUILD_ID = $${VERSION_MAJOR}.$$system(date +"%Y%m%d%H%M")
    }
    message("Generated BUILD_ID: $${BUILD_ID}")
}

DBUS_PROTOCOL_VERSION = 1

# Generate the version header file.
VERSION_TEMPLATE = $$PWD/src/version.h.in
genversion.input = VERSION_TEMPLATE
genversion.output = $$PWD/src/version.h
genversion.commands = @echo Building ${QMAKE_FILE_OUT} && \
    python3 $$PWD/scripts/utils/make_template.py ${QMAKE_FILE_IN} \
        -o ${QMAKE_FILE_OUT} -k @CMAKE_PROJECT_VERSION@=$${VERSION} -k @BUILD_ID@=$${BUILD_ID}
genversion.CONFIG = target_predeps no_link
genversion.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += genversion

