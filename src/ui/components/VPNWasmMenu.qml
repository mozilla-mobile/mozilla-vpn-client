/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.12
import "../themes/themes.js" as Theme

VPNFlickable {
    flickContentHeight: col.childrenRect.height
    anchors.fill: parent

    Column {
        id: col

        spacing: 0
        width: parent.width * .83
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Theme.rowHeight / 2

       VPNBoldLabel {
            id: viewsLabel

            text: "Views"
            font.pixelSize: Theme.windowMargin
            width: parent.width
            height: Theme.rowHeight
            leftPadding: 6
            verticalAlignment: Text.AlignVCenter

            Rectangle {
                height: 2
                width: parent.width
                color: "#eee"
                radius: 4
                anchors.bottom: parent.bottom
            }
        }

       VPNVerticalSpacer {
           height: Theme.windowMargin / 2
        }


        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Main"
            onClicked: mainStackView.replace("../states/StateMain.qml", StackView.Immediate)
        }

        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Subscription Needed (IAP) - iOS"
            onClicked: mainStackView.replace("../states/StateSubscriptionNeeded.qml", {wasmView: true}, StackView.Immediate)
        }

        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Device list: Max number of devices reached"
            onClicked: mainStackView.replace("../views/ViewDevices.qml", {wasmView: true}, StackView.Immediate)
        }

        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Update Required"
            onClicked: mainStackView.replace("../views/ViewUpdate.qml", {state: "required"}, StackView.Immediate)
        }

        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Update Recommended"
            onClicked: mainStackView.replace("../views/ViewUpdate.qml", {state: "recommended"}, StackView.Immediate)
        }

        // TODO
        /*
            -- Main ( VPN Connecting )
            -- Main ( VPN Confirming )
            -- Main ( VPN Switching )
            -- Main ( VPN Connection Unstable )
            -- Main ( VPN No Connection)
        */


        VPNVerticalSpacer {
            height: Theme.windowMargin
         }

        VPNBoldLabel {
             text: "Other"
             font.pixelSize: 16
             width: parent.width
             height: Theme.rowHeight
             leftPadding: 6
             verticalAlignment: Text.AlignVCenter

             Rectangle {
                 height: 2
                 width: parent.width
                 color: "#eee"
                 radius: 4
                 anchors.bottom: parent.bottom
             }
         }

        VPNVerticalSpacer {
            height: Theme.windowMargin / 2
         }

        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Alerts"
            onClicked: mainStackView.replace("../components/VPNWasmAlerts.qml", StackView.Immediate)
        }

        VPNRemoveDevicePopup {
            z: 4
            id: popup
            deviceName: "<DevicName>"
        }

        VPNWasmMenuButton {
            Layout.fillWidth: true
            text: "Device list: device removal confirmation"
            onClicked: popup.open()
        }

    }
}
