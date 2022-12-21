# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += \
        $$PWD/constants.cpp \
        $$PWD/cryptosettings.cpp \
        $$PWD/curve25519.cpp \
        $$PWD/feature.cpp \
        $$PWD/fontloader.cpp \
        $$PWD/hacl-star/Hacl_Chacha20.c \
        $$PWD/hacl-star/Hacl_Chacha20Poly1305_32.c \
        $$PWD/hacl-star/Hacl_Curve25519_51.c \
        $$PWD/hacl-star/Hacl_Poly1305_32.c \
        $$PWD/hawkauth.cpp \
        $$PWD/hkdf.cpp \
        $$PWD/ipaddress.cpp \
        $$PWD/leakdetector.cpp \
        $$PWD/logger.cpp \
        $$PWD/loghandler.cpp \
        $$PWD/rfc/rfc1112.cpp \
        $$PWD/rfc/rfc1918.cpp \
        $$PWD/rfc/rfc4193.cpp \
        $$PWD/rfc/rfc4291.cpp \
        $$PWD/rfc/rfc5735.cpp \
        $$PWD/settingsholder.cpp \
        $$PWD/taskscheduler.cpp \
        $$PWD/temporarydir.cpp \
        $$PWD/versionutils.cpp

HEADERS += \
        $$PWD/constants.h \
        $$PWD/cryptosettings.h \
        $$PWD/curve25519.h \
        $$PWD/env.h \
        $$PWD/feature.h \
        $$PWD/fontloader.h \
        $$PWD/hawkauth.h \
        $$PWD/hkdf.h \
        $$PWD/ipaddress.h \
        $$PWD/leakdetector.h \
        $$PWD/logger.h \
        $$PWD/loghandler.h \
        $$PWD/rfc/rfc1112.h \
        $$PWD/rfc/rfc1918.h \
        $$PWD/rfc/rfc4193.h \
        $$PWD/rfc/rfc4291.h \
        $$PWD/rfc/rfc5735.h \
        $$PWD/settingsholder.h \
        $$PWD/task.h \
        $$PWD/taskscheduler.h \
        $$PWD/temporarydir.h \
        $$PWD/versionutils.h

# Signal handling for unix platforms
unix {
    SOURCES += $$PWD/signalhandler.cpp
    HEADERS += $$PWD/signalhandler.h
}

ios {
    OBJECTIVE_SOURCES += \
        shared/platforms/macos/macoscryptosettings.mm
}
