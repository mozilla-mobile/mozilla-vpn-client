# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macos|win* {
    message(Enabling the webextension support)

    DEFINES += MVPN_WEBEXTENSION

    SOURCES += \
            $$PWD/../server/serverconnection.cpp \
            $$PWD/../server/serverhandler.cpp
    HEADERS += \
            $$PWD/../server/serverconnection.h \
            $$PWD/../server/serverhandler.h

    include($$PWD/extensionbridge.pri)
}
