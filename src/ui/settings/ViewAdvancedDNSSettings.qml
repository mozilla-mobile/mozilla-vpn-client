/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    property string settingId_default: "default"
    property string settingId_local: "local"
    property string settingId_adblock: "adblock"
    property string settingId_antiTracking: "antiTracking"
    property string settingId_adblockAndAntiTracking: "adblockAndAntiTracking"

    id: root

    StackView.onDeactivating: root.opacity = 0

    Behavior on opacity {
        PropertyAnimation {
            duration: 100
        }
    }

    VPNMenu {
        id: menu
        objectName: "settingsAdvancedDNSSettingsBackButton"

        title: "DNS Settings"
        isSettingsView: true
    }

    VPNTabNavigation {
        // hacks to circumvent the fact that we can't send
        // "scripts" as property values through ListModel/ListElement

        id: tabs
        width: root.width
        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.height - menu.height

        tabList: ListModel {
              id: tabButtonList
              ListElement {
                  buttonLabel: "Default"
              }
              ListElement {
                  buttonLabel: "Advanced"
              }
        }

        ButtonGroup {
            id: radioButtonGroup
        }

        stackContent: [
            VPNViewDNSSettings {
                settingsListModel: ListModel {
                    ListElement {
                        settingId: "default"
                        //% "Use default DNS"
                        settingTitle: qsTrId("vpn.advancedDNSSettings.gateway")
                        //% "Automatically use Mozilla VPN-protected DNS"
                        settingDescription: qsTrId("vpn.advancedDNSSettings.gateway.description")
                        showDNSInput: false
                    }
                }
            },
            VPNViewDNSSettings {
                settingsListModel: ListModel {
                    ListElement {
                        settingId: "adblock"
                        settingTitle: "Use adblock DNS"
                        settingDescription: "Block internet ads with VPN"
                        showDNSInput: false
                    }
                    ListElement {
                        settingId: "antiTracking"
                        settingTitle: "Use anti-tracking DNS"
                        settingDescription: "Block harmful domains with VPN"
                        showDNSInput: false
                    }
                    ListElement {
                        settingId: "adblockAndAntiTracking"
                        settingTitle: "Use adblock and anti-tracking DNS"
                        settingDescription: "Block internet ads and harmful domains with VPN"
                        showDNSInput: false
                    }
                    ListElement {
                        settingId: "local"
                        //% "Use local DNS"
                        settingTitle: qsTrId("vpn.advancedDNSSettings.localDNS")
                        //% "Resolve website domain names using a DNS in your local network"
                        settingDescription: qsTrId("vpn.advancedDNSSettings.localDNS.resolveWebsiteDomainNames")
                        showDNSInput: true
                    }
                }
            }

        ]
    }
}

