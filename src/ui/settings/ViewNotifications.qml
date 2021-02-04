/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: vpnFlickable

    flickContentHeight: columnLayout.height + menu.height
    width: window.width

    VPNMenu {
        id: menu

        //% "Notifications"
        title: qsTrId("vpn.settings.notifications")
        isSettingsView: true
    }

    ColumnLayout {
        id: columnLayout
        anchors.top: menu.bottom
        width: vpnFlickable.width - Theme.windowMargin
        spacing: 0

        /* TODO
        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            //% "Unsecured network alert"
            labelText: qsTrId("vpn.settings.unsecuredNetworkAlert")
            //% "Get notified if you connect to an unsecured Wi-Fi network"
            subLabelText: qsTrId("vpn.settings.unsecuredNetworkAlert.description")
            isChecked: VPNSettings.unsecuredNetworkAlert
            isEnabled: isVPNOff
            showDivider: isVPNOff
            onClicked: VPNSettings.unsecuredNetworkAlert = !VPNSettings.unsecuredNetworkAlert
        }
        */

        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)

            //% "Guest Wi-Fi portal alert"
            labelText: qsTrId("vpn.settings.guestWifiAlert")
            //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
            subLabelText: qsTrId("vpn.settings.guestWifiAlert.description")
            isChecked: VPNSettings.captivePortalAlert
            isEnabled: isVPNOff
            showDivider: isVPNOff
            onClicked: VPNSettings.captivePortalAlert = !VPNSettings.captivePortalAlert
        }

        VPNCheckBoxAlert {
        }

    }

}
