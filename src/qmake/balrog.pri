# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macos|win* {
    message(Balrog enabled)
    DEFINES += MVPN_BALROG

    SOURCES += $$PWD/../update/balrog.cpp
    HEADERS += $$PWD/../update/balrog.h

    # TODO: remove this if-stmt
    !win* {
        GO_MODULES = $$PWD/../../balrog/api.go
    }
}
