/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

Item {
   // TODO
   // This view shows the delete request. There is the list of attached clients.

    Text {
        id: msg
        text: "A" + VPNAuthInApp.attachedClients
        anchors.top: parent.top
    }

    VPNButton {
        anchors.top: msg.bottom
        anchors.bottomMargin: 24
        text: "Delete" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.deleteAccount()
    }
}
