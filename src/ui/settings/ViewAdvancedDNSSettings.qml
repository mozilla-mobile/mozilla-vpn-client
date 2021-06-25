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
        objectName: "settingsAdvancedDNSSettingsBackButton"

        //% "Advanced DNS Settings"
        title: qsTrId("vpn.settings.advancedDNSSettings.title")
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
            Glean.sample.dnsSettingsViewOpened.record();
        }

        Column {
            id: col
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: 18
            spacing: Theme.windowMargin

            // TODO: this should be a radiobox
            VPNCheckBoxRow {
                id: useGatewayDNS
                objectName: "gatewayDNSEnabled"
                width: parent.width - Theme.windowMargin


                //% "Use default DNS"
                labelText: qsTrId("vpn.advancedDNSSettings.gateway")
                //% "Automatically use Mozilla VPN-protected DNS"
                subLabelText: qsTrId("vpn.advancedDNSSettings.gateway.description")
                isChecked: VPNSettings.useGatewayDNS
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.useGatewayDNS = true
                    }
                }
            }

            // TODO: this should be a radiobox
            VPNCheckBoxRow {
                id: useLocalDNS
                objectName: "settingMozillaDNSEnabled"
                width: parent.width - Theme.windowMargin

                //% "Use local DNS"
                labelText: qsTrId("vpn.advancedDNSSettings.localDNS")
                //% "Resolve website domain names using a DNS in your local network"
                subLabelText: qsTrId("vpn.advancedDNSSettings.localDNS.resolveWebsiteDomainNames")
                isChecked: (!VPNSettings.useGatewayDNS)
                showDivider: false
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.useGatewayDNS = false
                    }
                }
            }

            VPNTextInput{
                id: ipInput

                isEnabled: !VPNSettings.useGatewayDNS && vpnFlickable.vpnIsOff
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

            VPNCheckBoxAlert {
                visible: !vpnFlickable.vpnIsOff

                //% "VPN must be off to edit these settings"
                //: Associated to a group of settings that require the VPN to be disconnected to change
                errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
            }
        }
    }
}

