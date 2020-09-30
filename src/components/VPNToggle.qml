/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import Mozilla.VPN 1.0

Rectangle {
    state: VPNController.state

    states: [
        State {
            name: VPNController.StateInitializing
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }
            PropertyChanges {
                target: toggle
                color: "#9E9E9E"
            }
        },
        State {
            name: VPNController.StateOff
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }
            PropertyChanges {
                target: toggle
                color: "#9E9E9E"
            }
        },
        State {
            name: VPNController.StateConnecting
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: "#998DB2"
            }
            PropertyChanges {
                target: toggle
                color: "#387E8A"
            }
        },
        State {
            name: VPNController.StateOn
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
            }
            PropertyChanges {
                target: toggle
                color: "#3FE1B0"
            }
        },
        State {
            name: VPNController.StateDisconnecting
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }
            PropertyChanges {
                target: toggle
                color: "#CECECE"
            }
        },
        State {
            name: VPNController.StateSwitching
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: "#998DB2"
            }
            PropertyChanges {
                target: toggle
                color: "#387E8A"
            }
        },
        State {
            name: VPNController.StateDeviceLimit
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }
            PropertyChanges {
                target: toggle
                color: "#E7E7E7"
            }
        }
    ]
    transitions: [
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    target: cursor
                    property: "anchors.leftMargin"
                    duration: 200
                }
                ColorAnimation {
                    targets: [toggle, cursor]
                    duration: 300
                }
            }
        }
    ]

    id: toggle
    height: 32
    width: 60
    radius: 16


    Rectangle {
        id: toggleHoverOutline
        color: "transparent"
        border.width: 5
        border.color: "#C2C2C2"
        opacity: {
            if (mouseArea.pressed &&
                (VPNController.state === VPNController.StateOn ||
                VPNController.state === VPNController.StateOff)) {
                    return 0.3
            }
            if (mouseArea.containsMouse) {
                return 0.2
            }
            return 0
        }
        anchors.fill: toggle
        anchors.margins: -5
        radius: toggleHoverOutline.height / 2
        antialiasing: true

        Behavior on opacity {
            PropertyAnimation {
                duration:200
            }
        }

    }

    Rectangle {
        id: cursor
        height: 24
        width: 24
        radius: 12
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 4
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (VPNController.state === VPNController.StateDeviceLimit ||
                VPNController.state === VPNController.StateInitializing) {
                return;
            }
            if (VPNController.state !== VPNController.StateOff) {
                VPNController.deactivate()
            } else {
                VPNController.activate()
            }
        }
    }
}
