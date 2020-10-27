/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Flickable {
    id: mainView

    width: parent.width
    contentWidth: parent.width
    contentHeight: alertBox.isUpdateAlert ? (parent.height + Theme.windowMargin + alertBox.height) : parent.height
    boundsBehavior: Flickable.StopAtBounds

    Item {
        id: mobileHeader

        height: 40
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin / 2
        visible: parent.height > Theme.verticalBreakPoint1

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            VPNIcon {
                source: "../resources/logo-tray.svg"
                sourceSize.height: 18
                sourceSize.width: 18
            }

            VPNBoldLabel {
                //% "Mozilla VPN"
                text: qsTr("MozillaVPN")
                color: "#000000"
                anchors.verticalCenter: parent.verticalCenter
            }

        }

    }

    VPNAlert {
        id: alertBox

        state: VPN.updateRecommended ? "recommended" : ""
        alertType: "update"
        alertColor: Theme.blueButton
        visible: state === "recommended"
        //% "New version is available."
        alertText: qsTrId("vpn.updates.newVersionAvailable")
        //% "Update now"
        alertLinkText: qsTrId("vpn.updates.updateNow")
        y: Theme.windowMargin
        width: parent.width - (Theme.windowMargin * 2)
    }

    Item {
        height: parent.contentHeight
        width: parent.width
        y: {
            let yDistance = 0;
            if (alertBox.visible)
                yDistance += alertBox.height + Theme.windowMargin;

            if (mobileHeader.visible)
                yDistance += mobileHeader.height;

            return yDistance;
        }

        VPNDropShadow {
            anchors.fill: box
            source: box
        }

        VPNControllerView {
            id: box
        }

        VPNControllerServer {
            id: serverInfo

            onClicked: stackview.push("ViewServers.qml")
            y: box.y + box.height + Theme.iconSize
        }

        VPNControllerDevice {
            anchors.top: serverInfo.bottom
            anchors.topMargin: 8
            onClicked: stackview.push("ViewDevices.qml")
        }

        Behavior on y {
            PropertyAnimation {
                duration: 200
                easing.type: Easing.OutCurve
            }

        }

    }

}
