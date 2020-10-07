/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Network settings")
        isSettingsView: true
    }

    ColumnLayout {
        anchors.top: menu.bottom
        Layout.preferredWidth: parent.width
        spacing: 0

        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            labelText: qsTr("IPv6")
            subLabelText: qsTr("Push the internet forward with the latest version of the Internet Protocol")
            isChecked: (VPNSettings.ipv6Enabled)
            isEnabled: isVPNOff
            showDivider: isVPNOff

            onClicked: VPNSettings.ipv6Enabled = !VPNSettings.ipv6Enabled
        }

        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            id: localNetwork
            labelText: qsTr("Local network access")
            subLabelText: qsTr("Access printers, streaming sticks and all other devices on your local network")
            isChecked: (VPNSettings.localNetworkAccess)
            isEnabled: isVPNOff
            showDivider: isVPNOff

            onClicked: VPNSettings.localNetworkAccess = !VPNSettings.localNetworkAccess
        }

        VPNCheckBoxAlert { }
    }

    ScrollBar.vertical: ScrollBar {}
}
