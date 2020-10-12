/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"

Flickable {
    VPNMenu {
        id: menu
        //% "Notifications"
        title: qsTrId("notifications")
        isSettingsView: true
    }

    ColumnLayout {
        anchors.top: menu.bottom
        Layout.preferredWidth: parent.width
        spacing: 0

        /* TODO
        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            //% "Unsecured network alert"
            labelText: qsTrId("unsecuredNetworkAlert")
            //% "Get notified if you connect to an unsecured Wi-Fi network"
            subLabelText: qsTrId("unsecuredNetworkAlert.description")
            isChecked: VPNSettings.unsecuredNetworkAlert
            isEnabled: isVPNOff
            showDivider: isVPNOff
            onClicked: VPNSettings.unsecuredNetworkAlert = !VPNSettings.unsecuredNetworkAlert
        }
        */

        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            //% "Guest Wi-Fi portal alert"
            labelText: qsTrId("guestWifiAlert")
            //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
            subLabelText: qsTrId("guestWifiAlert.description")
            isChecked: VPNSettings.captivePortalAlert
            isEnabled: isVPNOff
            showDivider: isVPNOff
            onClicked: VPNSettings.captivePortalAlert = !VPNSettings.captivePortalAlert
        }

        VPNCheckBoxAlert { }
    }

    ScrollBar.vertical: ScrollBar {}
}
