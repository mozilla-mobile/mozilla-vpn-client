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
            Glean.sample.networkSettingsViewOpened.record();
            if (!vpnIsOff) {
                Glean.sample.networkSettingsViewWarning.record();
            }
        }

        Column {
            id: col
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: 18
            spacing: Theme.windowMargin

            VPNCheckBoxRow {
                id: ipv6
                objectName: "settingIpv6Enabled"
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

            // TODO: to be removed when advanced DNS view is ready.
            VPNCheckBoxRow {
                id: useLocalDNS
                visible: VPNFeatureList.userDNSSupported
                objectName: "settingMozillaDNSEnabled"
                width: parent.width - Theme.windowMargin

                //% "Use local DNS"
                labelText: qsTrId("vpn.settings.useGatewayDNS")
                //% "Resolve websites using a DNS in your local network"
                subLabelText: qsTrId("vpn.settings.useGatewayDNS.description")
                isChecked: (!VPNSettings.useGatewayDNS)
                isEnabled: vpnFlickable.vpnIsOff
                showDivider: false
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.useGatewayDNS = !VPNSettings.useGatewayDNS
                    }
                }
            }

            // TODO: to be removed when advanced DNS view is ready.
            VPNTextInput{
                id: ipInput

                visible: (!VPNSettings.useGatewayDNS) && useLocalDNS.visible
                isEnabled: vpnFlickable.vpnIsOff
                leftPadding: 55

                width: parent.width - Theme.windowMargin
                height: 30
                value: VPNSettings.userDNS
                valueChanged: ip => {
                    if (ip === "") {
                        // If nothing is entered, thats valid too. We will ignore the value later.
                        ipInput.valueInvalid = false;
                        VPNSettings.userDNS = ip
                        return;
                    }

                    switch(VPNSettings.validateUserDNS(ip)) {
                    case VPNSettings.UserDNSOK:
                        ipInput.valueInvalid = false;
                        if (ip !== VPNSettings.userDNS) {
                            VPNSettings.userDNS = ip
                        }
                        break;

                    // Now bother user if the ip is invalid :)
                    case VPNSettings.UserDNSInvalid:
                        //% "Invalid IP address"
                        ipInput.error = qsTrId("vpn.settings.userDNS.invalid")
                        ipInput.valueInvalid = true;
                        break;

                    case VPNSettings.UserDNSNotIPv4:
                        //% "We currently support only IPv4 IP addresses"
                        ipInput.error = qsTrId("vpn.settings.userDNS.notIPv4")
                        ipInput.valueInvalid = true;
                        break;

                    case VPNSettings.UserDNSOutOfRange:
                        //% "Out of range IP address"
                        ipInput.error = qsTrId("vpn.settings.userDNS.outOfRange")
                        ipInput.valueInvalid = true;
                        break;
                    }
                }
            }

            // TODO: this needs to be fixed.
            VPNSettingsItem {
                objectName: "advancedDNSSettings"

                //% "Advanced DNS Settings"
                settingTitle: qsTrId("vpn.settings.networking.advancedDNSSettings")
                imageLeftSrc: "../resources/settings.svg"
                imageRightSrc: "../resources/chevron.svg"
                onClicked: settingsStackView.push("../settings/ViewAdvancedDNSSettings.qml")
                visible: false
                // TODO: visible: VPNFeatureList.userDNSSupported
                // TODO: isEnabled: vpnFlickable.vpnIsOff
            }

            VPNCheckBoxAlert {
                visible: !vpnFlickable.vpnIsOff

                //% "VPN must be off to edit these settings"
                //: Associated to a group of settings that require the VPN to be disconnected to change
                errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
            }
        }
    }
}
