

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: viewRoot

    VPNMenu {
        id: menu
        objectName: "settingsNotificationsBackButton"
        //% "Tracking- and Ad-Protection"
        title: qsTrId("vpn.settings.protections.title")
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        anchors.top: menu.bottom
        anchors.left: viewRoot.left
        height: viewRoot.height - menu.height
        width: viewRoot.width
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
        flickContentHeight: holder.height

        ColumnLayout {
            id: holder
            anchors.top: menu.bottom
            anchors.topMargin: Theme.windowMargin * 1.5
            anchors.horizontalCenter: parent.horizontalCenter
            width: vpnFlickable.width - Theme.windowMargin * 3.5
            spacing: 10

            VPNCheckBoxAlert {
                id: vpnOnAlert
                Layout.topMargin: Theme.windowMargin * 2
                visible: !vpnFlickable.vpnIsOff
                width: enableProtections.width
                //% "VPN must be off to edit App Permissions"
                //: Associated to a group of settings that require the VPN to be disconnected to change
                errorMessage: qsTrId(
                                  "vpn.settings.protectSelectedApps.vpnMustBeOff")
            }

            RowLayout {
                // Enable Protections Box
                id: enableProtections
                Layout.topMargin: Theme.windowMargin * 2
                spacing: Theme.windowMargin

                Rectangle {
                    id: rect
                    anchors.fill: enableProtections
                    anchors.topMargin: -12
                    anchors.bottomMargin: anchors.topMargin
                    anchors.leftMargin: -Theme.windowMargin
                    anchors.rightMargin: anchors.leftMargin
                    color: Theme.white
                    radius: 4
                    // It's filling enableProtections
                    // But for the layout calc it does not need to take space
                    Layout.preferredHeight: 0
                    Layout.preferredWidth: 0
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    VPNInterLabel {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        //% "Enable Tracking Protection & AD-Blocking"
                        text: qsTrId("vpn.settings.protections.enable")
                        color: Theme.fontColorDark
                        horizontalAlignment: Text.AlignLeft
                    }

                    VPNTextBlock {
                        Layout.fillWidth: true
                        //% "Block unwanted Connections to AD & Tracking Servers"
                        text: qsTrId("vpn.settings.protections.description")
                        visible: !!text.length
                    }
                }

                VPNSettingsToggle {
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: 45
                    checked: (!VPNSettings.useGatewayDNS)
                    enabled: vpnFlickable.vpnIsOff
                    toolTipTitle: qsTrId("vpn.settings.protections.enable")
                    onClicked: {
                        if (vpnFlickable.vpnIsOff) {
                            VPNSettings.useGatewayDNS = !VPNSettings.useGatewayDNS
                        }
                    }
                }
            }

            VPNBoldLabel {
                Layout.topMargin: 30
                width: parent.width
                //% "Select Block List"
                text: qsTrId("vpn.settings.protections.mode")
                visible: !VPNSettings.useGatewayDNS
            }
            VPNRadioDelegate {
                isHoverable: true
                //% "Block Ad-Servers"
                radioButtonLabelText: qsTrId(
                                          "vpn.settings.protections.mode.ads")
                accessibleName: radioButtonLabelText
                checked: VPNSettings.dnsProvider === VPNSettings.BlockAds
                visible: !VPNSettings.useGatewayDNS
                onClicked: {
                    VPNSettings.dnsProvider = VPNSettings.BlockAds
                }
            }
            VPNRadioDelegate {
                isHoverable: true
                //% "Block Trackers"
                radioButtonLabelText: qsTrId(
                                          "vpn.settings.protections.mode.trackers")
                accessibleName: radioButtonLabelText
                checked: VPNSettings.dnsProvider === VPNSettings.BlockTracking
                visible: !VPNSettings.useGatewayDNS
                onClicked: {
                    VPNSettings.dnsProvider = VPNSettings.BlockTracking
                }
            }
            VPNRadioDelegate {
                isHoverable: true
                //% "Block Trackers & Ads"
                radioButtonLabelText: qsTrId(
                                          "vpn.settings.protections.mode.all")
                accessibleName: radioButtonLabelText
                checked: VPNSettings.dnsProvider === VPNSettings.BlockAll
                visible: !VPNSettings.useGatewayDNS
                onClicked: {
                    VPNSettings.dnsProvider = VPNSettings.BlockAll
                }
            }
            VPNRadioDelegate {
                isHoverable: true
                //% "Block using a Custom DNS Service"
                radioButtonLabelText: qsTrId(
                                          "vpn.settings.protections.mode.custom")
                accessibleName: radioButtonLabelText
                checked: VPNSettings.dnsProvider === VPNSettings.Custom
                visible: !VPNSettings.useGatewayDNS
                onClicked: {
                    VPNSettings.dnsProvider = VPNSettings.Custom
                }
            }

            VPNTextInput {
                id: ipInput
                visible: !VPNSettings.useGatewayDNS
                         && VPNSettings.dnsProvider === VPNSettings.Custom
                isEnabled: vpnFlickable.vpnIsOff
                width: parent.width
                Layout.bottomMargin: ipInput.focus ? 500 : 0

                valueChanged:(ip)=>{
                  if(ip.length < 7){
                   // If we have less then 7 characters it's impossible to have a valid ip
                   // so we should not bother the user yet while the input has focus
                   ipInput.valueInavlid = false;
                   return;
                  }

                  // Now bother user if the ip is invalid :)
                  if(!VPNSettings.isValidCustomDNS(ip)){
                    ipInput.valueInavlid = true;

                    return;
                  }
                  ipInput.valueInavlid = false;
                  if(ip !== VPNSettings.customDNS){
                    VPNSettings.customDNS=ip
                  }
                }
            }
        }
    }
}
