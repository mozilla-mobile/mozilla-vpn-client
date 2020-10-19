/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0

Rectangle {
    id: listHeader

    property var pendingDeviceRemoval: false

    width: deviceList.width
    color: "transparent"
    state: "deviceLimitNotReached"
    states: [
        State {
            name: "deviceLimitReached"
            when: deviceWrapper.state === "deviceLimit"

            PropertyChanges {
                target: listHeader
                height: 252
                opacity: 1
            }

        },
        State {
            name: "deviceLimitNotReached"
            when: deviceWrapper.state === "active"

            PropertyChanges {
                target: listHeader
                height: 8
                opacity: 0
                pendingDeviceRemoval: false
            }

        }
    ]
    transitions: [
        Transition {
            to: "deviceLimitNotReached"

            SequentialAnimation {
                PropertyAnimation {
                    property: "opacity"
                    duration: 200
                }

                PropertyAnimation {
                    property: "height"
                    duration: 300
                    easing.type: Easing.Linear
                }

            }

        }
    ]

    Rectangle {
        id: deviceLimitAlert

        anchors.fill: parent
        anchors.topMargin: 32
        color: "transparent"

        Image {
            id: alertImg

            source: "../resources/devicesLimit.svg"
            sourceSize.width: 80
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
        }

        VPNHeadline {
            id: alertHeadline

            anchors.top: alertImg.bottom
            anchors.topMargin: 24
            //% "Remove a device"
            text: qsTrId("vpn.devices.doDeviceRemoval")
        }

        VPNSubtitle {
            anchors.top: alertHeadline.bottom
            anchors.topMargin: 8
            //% "You've reached your limit. To install the VPN on this device, you'll need to remove one."
            text: qsTrId("vpn.devices.maxDevicesReached")
        }

    }

}
