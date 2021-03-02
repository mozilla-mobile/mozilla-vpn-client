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

    VPNCheckBoxRow {
        id: unsecuredNetworkAlert

        anchors.top: VPNFeatureList.captivePortalNotificationSupported ? captivePortalAlert.bottom : menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin
        visible: VPNFeatureList.unsecuredNetworkNotificationSupported

        //% "Unsecured network alert"
        labelText: qsTrId("vpn.settings.unsecuredNetworkAlert")
        //% "Get notified if you connect to an unsecured Wi-Fi network"
        subLabelText: qsTrId("vpn.settings.unsecuredNetworkAlert.description")

        isChecked: (VPNSettings.unsecuredNetworkAlert)
        isEnabled: vpnFlickable.vpnIsOff
        showDivider: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.unsecuredNetworkAlert = !VPNSettings.unsecuredNetworkAlert
            }
       }
    }

    VPNCheckBoxAlert {
        anchors.top: VPNFeatureList.unsecuredNetworkNotificationSupported ? unsecuredNetworkAlert.bottom : captivePortalAlert.bottom
        visible: !vpnFlickable.vpnIsOff

        //% "VPN must be off to edit these settings"
        //: Associated to a group of settings that require the VPN to be disconnected to change
        errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
    }
}
