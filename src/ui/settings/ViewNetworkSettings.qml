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

VPNFlickable {
    id: vpnFlickable
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

    VPNMenu {
        id: menu
        objectName: "settingsNetworkingBackButton"

        //% "Network settings"
        title: qsTrId("vpn.settings.networking")
        isSettingsView: true
    }

    ColumnLayout{
        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        VPNCheckBoxRow {
            id: ipv6
            objectName: "settingIpv6Enabled"
            width: parent.width

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

            //layout.topMargin: Theme.windowMargin
            width: parent.width
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
            width: parent.width
            visible: !vpnFlickable.vpnIsOff

            //% "VPN must be off to edit these settings"
            //: Associated to a group of settings that require the VPN to be disconnected to change
            errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
        }

        VPNCheckBoxRow {
            id: useGateWayDNS
            visible: VPNFeatureList.userDNSSupported
            objectName: "settingMozillaDNSEnabled"
            width: parent.width

            //% "Use VPN DNS"
            labelText: qsTrId("vpn.settings.useGatewayDNS")
            //% "Hide what websites you're visiting Mozillas Anonymous DNS-Service"
            subLabelText: qsTrId("vpn.settings.useGatewayDNS.description")
            isChecked: (VPNSettings.useGatewayDNS)
            isEnabled: vpnFlickable.vpnIsOff
            showDivider: isChecked
            onClicked: {
                if (vpnFlickable.vpnIsOff) {
                    VPNSettings.useGatewayDNS = !VPNSettings.useGatewayDNS
                }
           }
        }

        VPNTextInput{
            id: ipInput
            visible: !useGateWayDNS.isChecked && useGateWayDNS.visible
            isEnabled: vpnFlickable.vpnIsOff
            Layout.leftMargin: 55
            width: parent.width - Layout.leftMargin
            height: 30
            //% "DNS Server to use:"
            labelText: qsTrId("vpn.settings.userDNS.header")
            //% "Enter the DNS Server you would like to use. Please note that the connection to the Server won't use the VPN"
            subLabelText: qsTrId("vpn.settings.userDNS.description")


            valueChanged:(ip)=>{
              if(ip.length < 7){
               // If we have less then 7 characters it's impossible to have a valid ip
               // so we should not bother the user yet while the input has focus
               ipInput.valueInavlid = false;
               return;
              }

              // Now bother user if the ip is invalid :)
              if(!VPNSettings.isValidUserDNS(ip)){
                ipInput.valueInavlid = true;

                return;
              }
              ipInput.valueInavlid = false;
              if(ip !== VPNSettings.userDNS){
                VPNSettings.userDNS=ip
              }
            }
            value: VPNSettings.userDNS


        }

    }







}
