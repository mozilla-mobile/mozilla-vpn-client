# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += \
        $$PWD/collator.cpp \
        $$PWD/constants.cpp \
        $$PWD/cryptosettings.cpp \
        $$PWD/curve25519.cpp \
        $$PWD/feature.cpp \
        $$PWD/filterproxymodel.cpp \
        $$PWD/fontloader.cpp \
        $$PWD/glean/gleandeprecated.cpp \
        $$PWD/hacl-star/Hacl_Chacha20.c \
        $$PWD/hacl-star/Hacl_Chacha20Poly1305_32.c \
        $$PWD/hacl-star/Hacl_Curve25519_51.c \
        $$PWD/hacl-star/Hacl_Poly1305_32.c \
        $$PWD/hawkauth.cpp \
        $$PWD/hkdf.cpp \
        $$PWD/ipaddress.cpp \
        $$PWD/itempicker.cpp \
        $$PWD/languagei18n.cpp \
        $$PWD/leakdetector.cpp \
        $$PWD/localizer.cpp \
        $$PWD/logger.cpp \
        $$PWD/loghandler.cpp \
        $$PWD/models/licensemodel.cpp \
        $$PWD/networkmanager.cpp \
        $$PWD/networkrequest.cpp \
        $$PWD/qmlengineholder.cpp \
        $$PWD/qmlpath.cpp \
        $$PWD/rfc/rfc1112.cpp \
        $$PWD/rfc/rfc1918.cpp \
        $$PWD/rfc/rfc4193.cpp \
        $$PWD/rfc/rfc4291.cpp \
        $$PWD/rfc/rfc5735.cpp \
        $$PWD/settingsholder.cpp \
        $$PWD/signature.cpp \
        $$PWD/simplenetworkmanager.cpp \
        $$PWD/taskscheduler.cpp \
        $$PWD/temporarydir.cpp \
        $$PWD/theme.cpp \
        $$PWD/urlopener.cpp \
        $$PWD/versionutils.cpp

HEADERS += \
        $$PWD/collator.h \
        $$PWD/constants.h \
        $$PWD/cryptosettings.h \
        $$PWD/curve25519.h \
        $$PWD/env.h \
        $$PWD/feature.h \
        $$PWD/filterproxymodel.h \
        $$PWD/fontloader.h \
        $$PWD/glean/gleandeprecated.h \
        $$PWD/hawkauth.h \
        $$PWD/hkdf.h \
        $$PWD/ipaddress.h \
        $$PWD/itempicker.h \
        $$PWD/languagei18n.h \
        $$PWD/leakdetector.h \
        $$PWD/localizer.h \
        $$PWD/logger.h \
        $$PWD/loghandler.h \
        $$PWD/models/licensemodel.h \
        $$PWD/networkmanager.h \
        $$PWD/networkrequest.h \
        $$PWD/qmlengineholder.h \
        $$PWD/qmlpath.h \
        $$PWD/rfc/rfc1112.h \
        $$PWD/rfc/rfc1918.h \
        $$PWD/rfc/rfc4193.h \
        $$PWD/rfc/rfc4291.h \
        $$PWD/rfc/rfc5735.h \
        $$PWD/settingsholder.h \
        $$PWD/signature.h \
        $$PWD/simplenetworkmanager.h \
        $$PWD/task.h \
        $$PWD/taskscheduler.h \
        $$PWD/temporarydir.h \
        $$PWD/theme.h \
        $$PWD/urlopener.h \
        $$PWD/versionutils.h

# Signal handling for unix platforms
unix {
    SOURCES += $$PWD/signalhandler.cpp
    HEADERS += $$PWD/signalhandler.h
}

ios {
    OBJECTIVE_SOURCES += \
        shared/platforms/macos/macoscryptosettings.mm \
        shared/platforms/ios/ioscommons.mm

    OBJECTIVE_HEADERS += \
        shared/platforms/ios/ioscommons.h
}

RESOURCES += shared/resources/license.qrc
