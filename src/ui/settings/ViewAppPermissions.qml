/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    id: root

    VPNMenu {
        id: menu
        title: qsTrId("vpn.settings.appPermissions")
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        readonly property int defaultMargin: 18
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
        flickContentHeight: enableAppList.height + enableAppList.anchors.topMargin + (vpnOnAlert.visible ? vpnOnAlert.height : 0) + (disabledList.visible ? disabledList.height : 0) + (enabledList.visible ? enabledList.height : 0) + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        width: root.width
        Component.onCompleted: {
            VPNAppPermissions.requestApplist();
            Sample.appPermissionsViewOpened.record();
            if (!vpnIsOff) {
                Sample.appPermissionsViewWarning.record();
            }
         }

        VPNCheckBoxAlert {
            id: vpnOnAlert

            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin * 1
            visible: !vpnFlickable.vpnIsOff
            anchors.leftMargin: Theme.windowMargin
            width: enableAppList.width
            //% "VPN must be off to edit App Permissions"
            //: Associated to a group of settings that require the VPN to be disconnected to change
            errorMessage: qsTrId("vpn.settings.protectSelectedApps.vpnMustBeOff")
        }

        VPNDropShadow {
            anchors.fill: rect
            source: rect
            z: -2
        }

        Rectangle {
            id: rect
            anchors.fill: enableAppList
            anchors.topMargin: -12
            anchors.bottomMargin: anchors.topMargin
            anchors.leftMargin: -Theme.windowMargin
            anchors.rightMargin: anchors.leftMargin
            color: Theme.white
            radius: 4
        }

        RowLayout {
            id: enableAppList
            anchors.top: vpnOnAlert.visible ? vpnOnAlert.bottom : parent.top
            anchors.topMargin: Theme.windowMargin * 1.5
            anchors.horizontalCenter: parent.horizontalCenter
            width: vpnFlickable.width - Theme.windowMargin * 3.5
            spacing: Theme.windowMargin

            ColumnLayout {
                Layout.fillWidth: true
                VPNInterLabel {
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    //% "Protect specific apps"
                    text: qsTrId("vpn.settings.protectSelectedApps")
                    color: Theme.fontColorDark
                    horizontalAlignment: Text.AlignLeft
                }

                VPNTextBlock {
                    Layout.fillWidth: true
                    //% "Choose which apps should use the VPN"
                    text: qsTrId("vpn.settings.protectSelectedApps.description")
                    visible: !!text.length
                }
            }

            VPNSettingsToggle {
                Layout.preferredHeight: 24
                Layout.preferredWidth: 45
                checked: (VPNSettings.protectSelectedApps)
                enabled: vpnFlickable.vpnIsOff
                toolTipTitle: qsTrId("vpn.settings.protectSelectedApps")
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps
                    }
                }
            }
        }

        RowLayout {
            id: allAppsProtectedHint

            anchors.top: enableAppList.bottom
            anchors.topMargin: Theme.windowMargin * 2

            anchors.left: parent.left
            anchors.leftMargin: 18
            visible: !VPNSettings.protectSelectedApps && vpnFlickable.vpnIsOff

            Rectangle {
                color: "transparent"
                Layout.maximumHeight: infoMessage.lineHeight
                Layout.preferredWidth: 14
                Layout.rightMargin: 8
                Layout.alignment: Qt.AlignTop
                VPNIcon {
                    id: warningIcon

                    source: "../resources/connection-info.svg"
                    sourceSize.height: 14
                    sourceSize.width: 14
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            VPNTextBlock {
                id: infoMessage
                //% "VPN protects all apps by default"
                text: qsTrId("vpn.settings.protectSelectedApps.allAppsProtected")
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                verticalAlignment: Text.AlignTop
            }
        }

        VPNExpandableAppList{
            id: enabledList
            anchors.topMargin: 28
            anchors.top: enableAppList.bottom

            //% "Protected"
            //: Header for the list of apps protected by VPN
            header: qsTrId("vpn.settings.protected")
            //% "These apps will use the VPN"
            //: Description for the list of apps protected by VPN
            description: qsTrId("vpn.settings.protected.description")
            listModel: VPNAppPermissions.enabledApps
            onAction: ()=>{VPNAppPermissions.unprotectAll()}
            //% "Unprotect All"
            //: Label for the button to remove protection from all apps
            //: currently protected.
            actionText: qsTrId("vpn.settings.unprotectall")
            dividerVisible: true
        }

        VPNExpandableAppList{
            id: disabledList

            anchors.top: enabledList.bottom
            //% "Unprotected"
            //: Header for the list of apps not protected by VPN
            header: qsTrId("vpn.settings.unprotected")
            //% "These apps will not use the VPN"
            //: Description for the list of apps not protected by VPN
            description: qsTrId("vpn.settings.unprotected.description")
            listModel: VPNAppPermissions.disabledApps
            onAction: ()=>{VPNAppPermissions.protectAll()}
            //% "Protect All"
            //: Label for the button to add protection to all apps
            //: currently unprotected.
            actionText: qsTrId("vpn.settings.protectall")
        }
    }

}

