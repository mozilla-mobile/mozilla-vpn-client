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



    VPNBoldLabel {
        id: applistLabel
        visible: VPNSettings.protectSelectedApps

        anchors.top: vpnFlickable.vpnIsOff? enableAppList.bottom : vpnOnAlert.bottom
        anchors.topMargin: 28
        anchors.left: parent.left
        anchors.leftMargin: Theme.windowMargin
        width: parent.width
        //% "Installed Applications"
        //: Header for the installed Applications
        text: qsTrId("vpn.settings.installedApps")
        Accessible.role: Accessible.Heading
    }


    VPNList {
        id: applist
        visible: VPNSettings.protectSelectedApps && count > 0

        anchors.top: applistLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: defaultMargin
        anchors.rightMargin: defaultMargin
        width: parent.width - defaultMargin * 2
        height: contentItem.childrenRect.height + 40
        spacing: 26
        listName: applistLabel.text
        model: VPNAppPermissions


        delegate: VPNCheckBoxRow {
            labelText: appName
            subLabelText: appID
            isChecked: appIsEnabled
            isEnabled: vpnFlickable.vpnIsOff
            showDivider:false
            onClicked: VPNAppPermissions.flip(appID)
            visible: true

            width: applist.width
            anchors.left: parent.left
            anchors.topMargin: Theme.windowMargin

            Image {
                source: "image://app/"+appID
                width: 10
                height: 10
            }

        }
    }

    Image {
        id: spinner
        visible: VPNSettings.protectSelectedApps && applist.count == 0
        anchors.top: applistLabel.bottom
        anchors.topMargin: 32
        anchors.horizontalCenter: parent.horizontalCenter
        sourceSize.height: 80
        fillMode: Image.PreserveAspectFit
        source: "../resources/spinner.svg"

        ParallelAnimation {
            id: startSpinning

            running: true

            PropertyAnimation {
                target: spinner
                property: "opacity"
                from: 0
                to: 1
                duration: 300
            }

            PropertyAnimation {
                target: spinner
                property: "scale"
                from: 0.7
                to: 1
                duration: 300
            }

            PropertyAnimation {
                target: spinner
                property: "rotation"
                from: 0
                to: 360
                duration: 8000
                loops: Animation.Infinite
            }

        }

    }

}




/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
