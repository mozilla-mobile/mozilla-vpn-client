/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.5

import Mozilla.VPN 1.0

RoundButton {
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
            PropertyChanges {
                target: toggleButton
                //% "Turn VPN on"
                Accessible.name: qsTrId("action.VPN.on")
                //% "VPN is off"
                Accessible.description: qsTrId("vpnIsOff")
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
            PropertyChanges {
                target: toggleButton
                 //% "Turn VPN off"
                Accessible.name: qsTrId("action.VPN.off")
                //% "VPN is on"
                Accessible.description: qsTrId("vpnIsOn")
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
            PropertyChanges {
                target: toggleButton
                Accessible.name: "Disconnect"
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
                    target: cursor
                    duration: 200
                }
            }
        }
    ]

    id: toggleButton

    height: 32
    width: 60
    radius: 16

    background: Rectangle {
        id: toggle
        anchors.fill: toggleButton
        radius: toggleButton.radius

        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
    }

    focusPolicy: Qt.StrongFocus
    Keys.onSpacePressed: handleClick()
    Keys.onReturnPressed: handleClick()
    Accessible.onPressAction: handleClick()

    Rectangle {
        id: toggleOutline
        color: "transparent"
        border.color: "#C2C2C2"
        border.width: 5
        anchors.margins: -5
        anchors.fill: toggle
        radius: toggleOutline.height / 2
        antialiasing: true

        Behavior on opacity {
            PropertyAnimation {
                duration: toggleButton.activeFocus ? 0 : 200
            }
        }

        state: "state-default"
        states: [
            State {
                name: "state-default"
                PropertyChanges {
                    target: toggleOutline
                    opacity: toggleButton.activeFocus ? .3 : 0
                }
                PropertyChanges {
                    target: toggle
                    color: {
                        if (VPNController.state === VPNController.StateOn) {
                            return "#3FE1B0"
                        }

                        if (VPNController.state === VPNController.StateOff) {
                            return "#9E9E9E"
                        }
                        return toggle.color
                    }
                }
            },
            State {
                name: "state-hovering"
                PropertyChanges {
                    target: toggleOutline
                    opacity: .2
                }
                PropertyChanges {
                    target: toggle
                    color: {
                        if (VPNController.state === VPNController.StateOn) {
                            return "#3AD4B3"
                        }
                        if (VPNController.state === VPNController.StateOff) {
                            return "#6D6D6E"
                        }
                        return toggle.color
                    }
                }
            },
            State {
                name: "state-pressed"
                PropertyChanges {
                    target: toggleOutline
                    opacity: {
                        if (VPNController.state === VPNController.StateOn
                                || VPNController.state === VPNController.StateOff) {
                            return 0.3
                        }
                        return 0
                    }
                }
                PropertyChanges {
                    target: toggle
                    color: {
                        if (VPNController.state === VPNController.StateOn) {
                            return "#1CC5A0"
                        }
                        if (VPNController.state === VPNController.StateOff) {
                            return "#3D3D3D"
                        }
                        return toggle.color
                    }
                }
            }
        ]
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
        onEntered: toggleOutline.state = "state-hovering"
        onExited: toggleOutline.state = "state-default"
        onPressed: toggleOutline.state = "state-pressed"
        onClicked: toggleButton.handleClick()
    }

    function handleClick() {
        if (VPNController.state === VPNController.StateDeviceLimit
                || VPNController.state === VPNController.StateInitializing) {
            return toggleOutline.state = "state-default"
        }
        if (VPNController.state !== VPNController.StateOff) {
            return VPNController.deactivate()
        }
        return VPNController.activate()
    }
}
