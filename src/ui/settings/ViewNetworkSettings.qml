/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


Item {
    property string _appPermissionsTitle
    //% "Network settings"
    property string _menuTitle: qsTrId("vpn.settings.networking")

    id: root
    objectName: "settingsNetworkingBackButton"

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

        anchors.top: parent.top
        anchors.topMargin: 56
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

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: VPNTheme.theme.windowMargin
            }
            spacing: VPNTheme.theme.windowMargin

            VPNContextualAlerts {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: VPNTheme.theme.windowMargin
                    rightMargin: VPNTheme.theme.windowMargin
                }

                messages: [
                    {
                        type: "warning",
                        //% "VPN must be off to edit these settings"
                        //: Associated to a group of settings that require the VPN to be disconnected to change
                        message: qsTrId("vpn.settings.vpnMustBeOff"),
                        visible: VPNController.state !== VPNController.StateOff
                    }
                ]
            }

            VPNCheckBoxRow {
                id: localNetwork
                objectName: "settingLocalNetworkAccess"
                visible: VPNFeatureList.get("lanAccess").isSupported
                width: parent.width - VPNTheme.theme.windowMargin
                showDivider: true

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

            VPNCheckBoxRow {
                id: tunnelPort53
                objectName: "settingTunnelPort53"
                width: parent.width - VPNTheme.theme.windowMargin
                showDivider: true

                labelText: VPNl18n.SettingsTunnelPort53
                subLabelText: VPNl18n.SettingsTunnelPort53Description
                isChecked: (VPNSettings.tunnelPort53)
                isEnabled: vpnFlickable.vpnIsOff
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.tunnelPort53 = !VPNSettings.tunnelPort53
                    }
                }
            }

            Column {
                width: parent.width
                spacing: VPNTheme.theme.windowMargin  /2
                VPNSettingsItem {
                    objectName: "advancedDNSSettings"
                    anchors.left: parent.left
                    anchors.right: parent.right
                    width: parent.width - VPNTheme.theme.windowMargin

                    //% "Advanced DNS Settings"
                    settingTitle: qsTrId("vpn.settings.networking.advancedDNSSettings")
                    imageLeftSrc: "qrc:/ui/resources/settings-dark.svg"
                    imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                    onClicked: settingsStackView.push("qrc:/ui/settings/ViewAdvancedDNSSettings.qml")
                    visible: VPNFeatureList.get("customDNS").isSupported
                }

                VPNSettingsItem {
                    objectName: "appPermissions"
                    anchors.left: parent.left
                    anchors.right: parent.right
                    width: parent.width - VPNTheme.theme.windowMargin
                    settingTitle: _appPermissionsTitle
                    imageLeftSrc: "qrc:/ui/resources/settings/apps.svg"
                    imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                    onClicked: settingsStackView.push("qrc:/ui/settings/ViewAppPermissions.qml")
                    visible: VPNFeatureList.get("splitTunnel").isSupported
                }
            }
        }
    }
}
