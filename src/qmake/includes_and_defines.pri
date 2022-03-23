# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include($$PWD/../../version.pri)
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += BUILD_ID=\\\"$$BUILD_ID\\\"

!isEmpty(MVPN_EXTRA_USERAGENT) {
    DEFINES += MVPN_EXTRA_USERAGENT=\\\"$$MVPN_EXTRA_USERAGENT\\\"
}

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

INCLUDEPATH += \
            hacl-star \
            hacl-star/kremlin \
            hacl-star/kremlin/minimal \
            ../translations/generated \
            ../glean \
            ../lottie/lib \
            ../nebula

INCLUDEPATH += ../lottie/lib
DEPENDPATH  += $${INCLUDEPATH}
