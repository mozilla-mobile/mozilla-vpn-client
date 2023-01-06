# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include($$PWD/../../../../version.pri)
DEFINES += BUILD_QMAKE

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

INCLUDEPATH += \
            $$PWD/.. \
            $$PWD/../addons \
            $$PWD/../composer \
            $$PWD/../../../shared \
            $$PWD/../../../shared/glean \
            $$PWD/../../../shared/hacl-star \
            $$PWD/../../../shared/hacl-star/kremlin \
            $$PWD/../../../shared/hacl-star/kremlin/minimal \
            $$PWD/../../../../lottie/lib

DEPENDPATH  += $${INCLUDEPATH}
