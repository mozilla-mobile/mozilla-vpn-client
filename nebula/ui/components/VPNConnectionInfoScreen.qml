import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
    property bool isVisible: false
    property bool isAnimating: false
    property int transitionDuration: 750 // TODO: Experiment with duration

    id: root

    color: "pink"
    clip: true
    opacity: 0.5 // TODO: Set to `0`
    state: "hidden"
    states: [
        State {
            name: "hidden"
            when: !isVisible && !isAnimating

            PropertyChanges {
                target: root
                opacity: 0.5 // TODO: Set to `0`
                visible: true // TODO: Set to `false`
            }
        },
        State {
            name: "opening"
            when: isVisible && isAnimating

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        },
        State {
            name: "closing"
            when: !isVisible && isAnimating

            PropertyChanges {
                target: root
                opacity: 0.5 // TODO: Set to `0`
                visible: true
            }
        },
        State {
            name: "visible"
            when: isVisible && !isAnimating

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        }
    ]
    width: parent.width

    onIsVisibleChanged: () => {
        console.log("visible changing - start: ", isVisible);
        isAnimating = true;

        timer.setTimeout(function() {
            console.log("visible changing - end: ", isVisible);
            isAnimating = false;
        }, transitionDuration);
    }

    Behavior on opacity {
        NumberAnimation {
            target: root
            property: "opacity"
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
    }

    ColumnLayout {
        anchors.right: parent.right
        anchors.top: parent.top

        Text {
            text: "isVisible: " + root.isVisible
        }
        Text {
            text: "isAnimating: " + root.isAnimating
        }
        Text {
            text: "State: " + root.state
        }
    }

    Timer {
        id: timer

        function setTimeout(callback, timeoutDuration) {
            timer.interval = timeoutDuration;
            timer.repeat = false;
            timer.triggered.connect(callback);
            timer.triggered.connect(function release() {
                timer.triggered.disconnect(callback);
                timer.triggered.disconnect(release);
            });
            timer.start();
        }
    }

    // Header
    // - [] Closing button
    // - [] Restart button

    // Content
    VPNConnectionInfoContent {
    }
}