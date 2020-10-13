/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

RowLayout {
    id: turnVPNOffAlert
    visible: (VPNController.state !== VPNController.StateOff)
    Layout.topMargin: 12
    Layout.leftMargin: 56
    Layout.rightMargin: Theme.windowMargin
    spacing: 0

    VPNIcon {
        id: warningIcon
        source: "../resources/warning.svg"
        sourceSize.height: 14
        sourceSize.width: 14
        Layout.rightMargin: 8
        Layout.leftMargin: 4
    }
    Text {
        text: VPNSettings.localNetwork ?
                  //% "VPN must be off before enabling"
                  //: This is a Setting which requires the VPN to be disconnected to change state
                  qsTrId("vpn.turnOffAlert.enabling") :
                  //% "VPN must be off before disabling"
                  //: This is a Setting which requires the VPN to be disconnected to change state
                  qsTrId("vpn.turnOffAlert.disabling");
        color: Theme.red
        font.pixelSize: 13
        font.family: Theme.fontInterFamily
        wrapMode: Text.Wrap
        verticalAlignment: Qt.AlignVCenter
        width: 260
    }
}
