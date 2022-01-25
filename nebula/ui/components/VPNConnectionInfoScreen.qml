import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
    property bool isVisible: false
    property bool isAnimating: false
    property int transitionDuration: 1000

    id: root

    state: "hidden"
    states: [
        State {
            name: "hidden"
            when: !isVisible && !isAnimating

            PropertyChanges {
                target: root
                opacity: 0
            }
        },
        State {
            name: "opening"
            when: isVisible && isAnimating

            PropertyChanges {
                target: root
                opacity: 1
            }
        },
        State {
            name: "closing"
            when: !isVisible && isAnimating

            PropertyChanges {
                target: root
                opacity: 1
            }
        },
        State {
            name: "visible"
            when: isVisible && !isAnimating

            PropertyChanges {
                target: root
                opacity: 1
            }
        }
    ]

    Behavior on opacity {
        NumberAnimation {
            target: root
            property: "opacity"
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
    }

    color: VPNTheme.colors.primary
    opacity: 0.2
    width: parent.width
    onIsVisibleChanged: () => {
        console.log("visible changing - start: ", isVisible);
        isAnimating = true;

        timer.setTimeout(function() {
            console.log("visible changing - end: ", isVisible);
            isAnimating = false;
        }, transitionDuration);
    }

    ColumnLayout {
        x: 0
        y: 0

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

        function setTimeout(cb, delayTime) {
            timer.interval = delayTime;
            timer.repeat = false;
            timer.triggered.connect(cb);
            timer.triggered.connect(function release() {
                timer.triggered.disconnect(cb);
                timer.triggered.disconnect(release);
            });
            timer.start();
        }
    }
}