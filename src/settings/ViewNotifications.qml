/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Notifications")
        isSettingsView: true
    }

    ColumnLayout {
        anchors.top: menu.bottom
        Layout.preferredWidth: parent.width
        spacing: 0

        VPNCheckBoxRow {
            labelText: qsTr("Unsecured network alert")
            subLabelText: qsTr("Get notified if you connect to an unsecured Wi-Fi network")
            isChecked: true // TODO: add condition
            onClicked: {
                console.log("TODO: Handle click")
                isChecked = !isChecked
            }
        }

        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            labelText: qsTr("Guest Wi-Fi portal alert")
            subLabelText: qsTr("Get notified if a guest Wi-Fi portal is blocked due to VPN connection")
            isChecked: true // TODO: add condition
            isEnabled: isVPNOff
            showDivider: isVPNOff
            onClicked: {
                console.log("TODO: Handle click")
                isChecked = !isChecked
            }
        }

        VPNCheckBoxAlert { }
    }

    ScrollBar.vertical: ScrollBar {}
}
