/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: vpnFlickable
    readonly property int defaultMargin: 18
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

    flickContentHeight: menu.height + enableAppList.height + enableAppList.anchors.topMargin + (vpnOnAlert.visible ? vpnOnAlert.height : 0) + (disabledList.visible? (disabledList.height+disabledList.anchors.topMargin):0)+(enabledList.visible? (enabledList.height + enabledList.anchors.topMargin):0)+20

    Component.onCompleted: {
       VPNAppPermissions.requestApplist();
    }

    VPNMenu {
        id: menu
        title: qsTrId("vpn.settings.appPermissions")
        isSettingsView: true
    }

    VPNCheckBoxRow {
        id: enableAppList

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        //% "Protect specific apps"
        labelText: qsTrId("vpn.settings.protectSelectedApps")
        //% "Choose which apps should use the VPN"
        subLabelText: qsTrId("vpn.settings.protectSelectedApps.description")
        isChecked: (VPNSettings.protectSelectedApps)
        isEnabled: vpnFlickable.vpnIsOff
        showDivider: true
        onClicked: VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps

    }

    VPNCheckBoxAlert {
        id: vpnOnAlert
        anchors.top: enableAppList.bottom
        visible: !vpnFlickable.vpnIsOff

        //% "VPN must be off to edit App Permissions"
        //: Associated to a group of settings that require the VPN to be disconnected to change
        errorMessage: qsTrId("vpn.settings.protectSelectedApps.vpnMustBeOff")
    }


    RowLayout {
        id: allAppsProtectedHint
        anchors.top: enableAppList.bottom
        anchors.left: enableAppList.left
        anchors.leftMargin: 15
        anchors.topMargin: 15
        visible: !VPNSettings.protectSelectedApps && vpnFlickable.vpnIsOff
        Rectangle {
            color: "transparent"
            Layout.preferredHeight: infoMessage.lineHeight
            Layout.maximumHeight: infoMessage.lineHeight
            Layout.preferredWidth: 14
            Layout.rightMargin: 18
            Layout.leftMargin: 4
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
        }

    }

    VPNExpandableAppList{
        id: disabledList
        anchors.topMargin: 20
        anchors.top: vpnOnAlert.visible ? vpnOnAlert.bottom : enableAppList.bottom
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        width: vpnFlickable.width - 5 *2
        visible: VPNSettings.protectSelectedApps
        isEnabled: vpnIsOff

        //% "Unprotected"
        header: qsTrId("vpn.settings.unprotected")
        //% "These apps will not use the VPN"
        description: qsTrId("vpn.settings.unprotected.description")
        listModel: VPNAppPermissions.disabledApps
        onAction: ()=>{VPNAppPermissions.protectAll()}
        //% "Protect All"
        actionText: qsTrId("vpn.settings.protectall")
    }


    VPNExpandableAppList{
        id: enabledList
        anchors.topMargin: 20
        anchors.top: disabledList.bottom
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        width: vpnFlickable.width - 5 *2
        visible: VPNSettings.protectSelectedApps
        isEnabled: vpnIsOff

        //% "Protected"
        header: qsTrId("vpn.settings.protected")
        //% "These apps will use the VPN"
        description: qsTrId("vpn.settings.protected.description")
        listModel: VPNAppPermissions.enabledApps
        onAction: ()=>{VPNAppPermissions.unprotectAll()}
        //% "Unprotect All"
        actionText: qsTrId("vpn.settings.unprotectall")
    }
}
