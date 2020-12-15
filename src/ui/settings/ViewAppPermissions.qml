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

    flickContentHeight: menu.height+enableAppList.height+((!vpnFlickable.vpnIsOff)? vpnOnAlert.height:0) + enabledList.height + disabledList.height

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
        showDivider: false
        onClicked: VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps

    }

    VPNCheckBoxAlert {
        id: vpnOnAlert
        anchors.top: enableAppList.bottom
        visible: !vpnFlickable.vpnIsOff
    }

    VPNExpandableAppList{
        id: disabledList
        anchors.topMargin: 20
        anchors.top: vpnFlickable.vpnIsOff? enableAppList.bottom : vpnOnAlert.bottom

        anchors.leftMargin: 5
        anchors.rightMargin: 5
        width: vpnFlickable.width - 5 *2
        visible: VPNSettings.protectSelectedApps

        //% "Unprotected"
        header: qsTrId("vpn.settings.unprotected")
        //% "These apps will not use the VPN"
        description: qsTrId("vpn.settings.unprotected.desctiption")
        listModel: VPNAppPermissions.disabledApps
    }
    VPNExpandableAppList{
        id: enabledList
        anchors.topMargin: 20
        anchors.top: disabledList.bottom
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        width: vpnFlickable.width - 5 *2
        visible: VPNSettings.protectSelectedApps

        Behavior on y {
            PropertyAnimation {
                duration: 200
            }
        }
        //% "Protected"
        header: qsTrId("vpn.settings.protected")
        //% "These apps will use the VPN"
        description: qsTrId("vpn.settings.protected.desctiption")
        listModel: VPNAppPermissions.enabledApps
    }
}
