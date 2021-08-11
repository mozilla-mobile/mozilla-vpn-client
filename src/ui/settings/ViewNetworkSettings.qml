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

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    id: root

    VPNMenu {
        id: menu
        objectName: "settingsNetworkingBackButton"

        //% "Network settings"
        title: qsTrId("vpn.settings.networking")
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

        anchors.top: menu.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        height: root.height - menu.height
        flickContentHeight: col.childrenRect.height
        interactive: flickContentHeight > height

        Component.onCompleted: {
            Sample.networkSettingsViewOpened.record();
            if (!vpnIsOff) {
                Sample.networkSettingsViewWarning.record();
            }
        }

        Column {
            id: col
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
            spacing: Theme.windowMargin

            VPNCheckBoxAlert {
                id: alert
                //% "VPN must be off to edit these settings"
                //: Associated to a group of settings that require the VPN to be disconnected to change
                errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
            }

            VPNCheckBoxRow {
                id: ipv6
                objectName: "settingIpv6Enabled"
                width: parent.width - Theme.windowMargin
                showDivider: isEnabled

                //% "IPv6"
                labelText: qsTrId("vpn.settings.ipv6")
                //% "Push the internet forward with the latest version of the Internet Protocol"
                subLabelText: qsTrId("vpn.settings.ipv6.description")
                isChecked: (VPNSettings.ipv6Enabled)
                isEnabled: vpnFlickable.vpnIsOff
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.ipv6Enabled = !VPNSettings.ipv6Enabled
                    }
                }
            }

            VPNCheckBoxRow {
                id: localNetwork
                objectName: "settingLocalNetworkAccess"
                visible: VPNFeatureList.localNetworkAccessSupported
                width: parent.width - Theme.windowMargin
                showDivider: isEnabled

                //% "Local network access"
                labelText: qsTrId("vpn.settings.lanAccess")
                //% "Access printers, streaming sticks and all other devices on your local network"
                subLabelText: qsTrId("vpn.settings.lanAccess.description")
                isChecked: (VPNSettings.localNetworkAccess)
                isEnabled: vpnFlickable.vpnIsOff
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.localNetworkAccess = !VPNSettings.localNetworkAccess
                    }
                }
            }

            VPNSettingsItem {
                objectName: "advancedDNSSettings"
                anchors.left: parent.left
                anchors.right: parent.right
                width: parent.width - Theme.windowMargin


                settingTitle: "NeedString: DNS Settings"
                imageLeftSrc: "../resources/settings.svg"
                imageRightSrc: "../resources/chevron.svg"
                onClicked: settingsStackView.push("../settings/ViewAdvancedDNSSettings.qml")
                visible: VPNFeatureList.userDNSSupported
                enabled: vpnFlickable.vpnIsOff
                opacity: enabled ? 1 : .5
            }
        }

    }
}
