/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.VPN 1.0
import themes 0.1

VPNPopup {
    id: root

    anchors.centerIn: parent
    maxWidth: Theme.desktopAppWidth
    contentItem: Text {
        id: popupContentItem

        text: "Server unavailable"
    }

    Component.onCompleted: {
        root.open();
    }
}
