import QtQuick 2.0

import Mozilla.VPN 1.0

Rectangle {
    id: circle

    property var delay: 0
    property var duration: 1000
    property var minHeight: 100

    y: 90 - height / 2
    anchors.horizontalCenterOffset: 0
    anchors.horizontalCenter: parent.horizontalCenter
    border.width: 4
    border.color: "white"
    color: "transparent"
    width: height
    radius: height

    state: VPNController.state === VPNController.StateOn ? "active" : "inactive";

    states: [
        State {
            name: "active"
            StateChangeScript {
                   script: {
                       timer.running = true;
                   }
            }
            PropertyChanges {
                target: circle
                height: circle.minHeight
                opacity: 0
            }
            PropertyChanges {
                target: animation
                running: false
            }
        },

        State {
            name: "inactive"
            StateChangeScript {
                   script: {
                       timer.running = false;
                   }
            }
            PropertyChanges {
                target: circle
                height: circle.minHeight
                opacity: 0
            }
            PropertyChanges {
                target: animation
                running: false
            }
        }

    ]

    Timer {
        id: timer
        interval: circle.delay
        repeat: false
        onTriggered: {
            animation.running = true;
        }
    }

    ParallelAnimation {
        id: animation
        running: false
        loops: Animation.Infinite

        NumberAnimation {
            target: circle
            property: "height"
            from: circle.minHeight
            to: 250
            duration: circle.duration
            easing.type: Easing.Linear
        }

        PropertyAnimation {
            target: circle
            property: "opacity"
            from: 0.6
            to: 0
            duration: circle.duration
            easing.type: Easing.OutQuad
        }
    }
}
