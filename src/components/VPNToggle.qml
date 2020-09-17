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
            }
            PropertyChanges {
                target: toggle
                color: "#3FE1B0"
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
                color: "#9E9E9E"
            }
        },
        State {
            name: VPNController.StateSwitching
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
            to: VPNController.StateConnecting
            NumberAnimation {
                target: cursor
                property: "anchors.leftMargin"
                duration: 200
            }
            ColorAnimation {
                target: toggle
                property: "color"
                duration: 200
            }
        },

        Transition {
            to: VPNController.StateDisconnecting
            NumberAnimation {
                target: cursor
                property: "anchors.leftMargin"
                duration: 200
            }

            ColorAnimation {
                target: toggle
                property: "color"
                duration: 200
            }
        }
    ]

    id: toggle
    height: 32
    width: 60
    radius: 16

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
        anchors.fill: parent
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
