import QtQuick 2.0
import Mozilla.VPN 1.0

Rectangle {
    state: VPNController.state

    states: [
        State {
            name: VPNController.StateOff
        },
        State {
            name: VPNController.StateConnecting
        },
        State {
            name: VPNController.StateOn
        }
    ]

    transitions: [
        Transition {
            from: VPNController.StateOff
            to: VPNController.StateConnecting
            NumberAnimation {
                target: cursor
                property: "anchors.leftMargin"
                duration: 200
                from: 4
                to: 32
                alwaysRunToEnd: true
            }
            ColorAnimation {
                target: toggle
                property: "color"
                from: "#9E9E9E"
                to: "#3FE1B0"
                duration: 200
            }
        },

        Transition {
            from: VPNController.StateConnecting
            to: VPNController.StateOff
            NumberAnimation {
                target: cursor
                property: "anchors.leftMargin"
                duration: 200
                from: 32
                to: 4
                alwaysRunToEnd: true
            }

            ColorAnimation {
                target: toggle
                property: "color"
                from: "#3FE1B0"
                to: "#9E9E9E"
                duration: 200
            }
        }
    ]

    id: toggle
    height: 32
    width: 60
    radius: 16

    color: "#9E9E9E"

    Rectangle {
        id: cursor
        height: 24
        width: 24
        radius: 12
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.leftMargin: toggle.active ? 32 : 4
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (VPNController.state !== VPNController.StateOff) {
                VPNController.deactivate()
            } else {
                VPNController.activate()
            }
        }
    }

    Component.onCompleted: {
        console.log(VPNController)
    }
}
