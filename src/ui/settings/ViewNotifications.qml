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
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

    VPNMenu {
        id: menu

        //% "Notifications"
        title: qsTrId("vpn.settings.notifications")
        isSettingsView: true
    }

    VPNCheckBoxRow {
        id: captivePortalAlert

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin
        visible: VPNFeatureList.captivePortalNotificationSupported

        //% "Guest Wi-Fi portal alert"
        labelText: qsTrId("vpn.settings.guestWifiAlert")
        //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
        subLabelText: qsTrId("vpn.settings.guestWifiAlert.description")

        isChecked: (VPNSettings.captivePortalAlert)
        isEnabled: vpnFlickable.vpnIsOff
        showDivider: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.captivePortalAlert = !VPNSettings.captivePortalAlert
            }
       }
    }

    VPNCheckBoxAlert {
        anchors.top: captivePortalAlert.bottom
        visible: !vpnFlickable.vpnIsOff

        errorMessage: qsTrId("vpn.turnOffAlert.vpnMustBeOff")
    }
}
