# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

!defined(VERSION, var):VERSION = 2.9.0

VERSION_MAJOR = $$section(VERSION, ., 0, 0)
!defined(BUILD_ID) {
    BUILD_ID = $${VERSION_MAJOR}.$$system(date +"%Y%m%d%H%M")
}

DBUS_PROTOCOL_VERSION = 1
