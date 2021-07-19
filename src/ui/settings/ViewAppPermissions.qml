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

    //% "Search Apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add Application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")


    VPNMenu {
        id: menu
        title: qsTrId("vpn.settings.appPermissions2")
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
        flickContentHeight:  VPNSettings.protectSelectedApps ? enabledList.y + enabledList.implicitHeight + 100 : enabledList.y
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
            //% "VPN must be off to edit App Permissions"
            //: Associated to a group of settings that require the VPN to be disconnected to change
            errorMessage: qsTrId("vpn.settings.protectSelectedApps.vpnMustBeOff")
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
        }

        VPNDropShadow {
            anchors.fill: rect
            source: rect
            z: -2
        }

        Rectangle {
            id: rect
            anchors.fill: enableAppList
            anchors.topMargin: -Theme.windowMargin
            anchors.bottomMargin: anchors.topMargin
            anchors.leftMargin: -Theme.windowMargin
            anchors.rightMargin: anchors.leftMargin
            color: Theme.white
            radius: 4
        }

        RowLayout {
            id: enableAppList
            anchors.top: vpnOnAlert.visible ? vpnOnAlert.bottom : parent.top
            anchors.topMargin: Theme.windowMargin + (rect.anchors.topMargin *-1)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Theme.windowMargin * 2
            anchors.rightMargin: Theme.windowMargin * 2
            spacing: Theme.windowMargin

            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter
                VPNInterLabel {
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    //% "Protect all apps with VPN"
                    text: qsTrId("vpn.settings.protectAllApps")
                    color: Theme.fontColorDark
                    horizontalAlignment: Text.AlignLeft
                }

                VPNTextBlock {
                    Layout.fillWidth: true
                    //% "VPN protects all apps by default. Turn off to choose which apps Mozilla VPN should not protect."
                    text: qsTrId("vpn.settings.protectAllApps.description")
                    visible: !!text.length
                    width: undefined
                }
            }

            VPNSettingsToggle {
                Layout.preferredHeight: 24
                Layout.preferredWidth: 45
                checked: (!VPNSettings.protectSelectedApps)
                enabled: vpnFlickable.vpnIsOff
                toolTipTitle: qsTrId("vpn.settings.protectAllApps")
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps
                    }
                }
            }
        }

        VPNExpandableAppList {
            id: enabledList
            anchors.topMargin: 30
            anchors.top: enableAppList.bottom
            searchBarPlaceholder: searchApps

            //% "Exclude apps from VPN protection"
            //: Header for the list of apps protected by VPN
            header: qsTrId("vpn.settings.excludeTitle")
        }
    }
}
