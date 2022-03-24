# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

debug {
    # If in debug mode, set mvpn_debug flag too.
    CONFIG += mvpn_debug
}

mvpn_debug {
    message(MVPN Debug enabled)
    DEFINES += MVPN_DEBUG

    # This Flag will enable a qmljsdebugger on 0.0.0.0:1234
    CONFIG+=qml_debug
}
