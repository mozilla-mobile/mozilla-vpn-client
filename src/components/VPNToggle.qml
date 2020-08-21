import QtQuick 2.0

Rectangle {
    signal activate
    signal deactivate

    state: "inactive"

    states: [
        State {
            name: "active"
        },
        State {
            name: "inactive"
        }
    ]

    transitions: [
        Transition {
            from: "inactive"
            to: "active"
            NumberAnimation {
                target: cursor
                property: "anchors.leftMargin"
                duration: 200
                from: 4
                to: 32
                alwaysRunToEnd: true
            }
        },

        Transition {
            from: "active"
            to: "inactive"
            NumberAnimation {
                target: cursor
                property: "anchors.leftMargin"
                duration: 200
                from: 32
                to: 4
                alwaysRunToEnd: true
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
            if (toggle.state == "active") {
                toggle.state = "inactive"
                toggle.deactivate()
            } else {
                toggle.state = "active"
                toggle.activate()
            }
        }
    }
}
