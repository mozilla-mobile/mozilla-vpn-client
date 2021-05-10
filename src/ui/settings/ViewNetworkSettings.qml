/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme
import "/glean/load.js" as Glean

VPNFlickable {
    id: vpnFlickable
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

    Component.onCompleted: {
        Glean.sample.networkSettingsViewOpened.record();
        if (!vpnIsOff) {
            Glean.sample.networkSettingsViewVpnMustBeOff.record();
        }
     }

    VPNMenu {
        id: menu
        objectName: "settingsNetworkingBackButton"

        //% "Network settings"
        title: qsTrId("vpn.settings.networking")
        isSettingsView: true
    }

    VPNCheckBoxRow {
        id: ipv6
        objectName: "settingIpv6Enabled"

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        //% "IPv6"
        labelText: qsTrId("vpn.settings.ipv6")
        //% "Push the internet forward with the latest version of the Internet Protocol"
        subLabelText: qsTrId("vpn.settings.ipv6.description")
        isChecked: (VPNSettings.ipv6Enabled)
        isEnabled: vpnFlickable.vpnIsOff
        showDivider: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.ipv6Enabled = !VPNSettings.ipv6Enabled
            }
       }
    }

    VPNCheckBoxRow {
        id: localNetwork
        objectName: "settingLocalNetworkAccess"

        anchors.top: ipv6.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin
        visible: VPNFeatureList.localNetworkAccessSupported

        //% "Local network access"
        labelText: qsTrId("vpn.settings.lanAccess")
        //% "Access printers, streaming sticks and all other devices on your local network"
        subLabelText: qsTrId("vpn.settings.lanAccess.description")
        isChecked: (VPNSettings.localNetworkAccess)
        isEnabled: vpnFlickable.vpnIsOff
        showDivider: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.localNetworkAccess = !VPNSettings.localNetworkAccess
            }
       }
    }

    VPNCheckBoxAlert {
        anchors.top: localNetwork.visible ? localNetwork.bottom : ipv6.bottom
        visible: !vpnFlickable.vpnIsOff

        //% "VPN must be off to edit these settings"
        //: Associated to a group of settings that require the VPN to be disconnected to change
        errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
    }

}
