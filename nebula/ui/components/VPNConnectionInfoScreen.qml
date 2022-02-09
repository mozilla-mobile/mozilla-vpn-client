import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
    property bool isOpen: false
    property bool isAnimating: false
    property bool isLoading: false
    property int transitionDuration: 750

    id: root

    color: VPNTheme.colors.primary
    clip: true
    opacity: 0
    state: "closed"
    states: [
        State {
            name: "closed"
            when: !isOpen && !isAnimating

            PropertyChanges {
                target: root
                opacity: 0
                visible: false
            }
        },
        State {
            name: "opening"
            when: isOpen && isAnimating

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        },
        State {
            name: "closing"
            when: !isOpen && isAnimating

            PropertyChanges {
                target: root
                opacity: 0
                visible: true
            }
        },
        State {
            name: "open-loading"
             when: isOpen && !isAnimating && isLoading

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        },
        State {
            name: "open-ready"
            when: isOpen && !isAnimating && !isLoading

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        }
    ]
    width: parent.width

    onIsOpenChanged: () => {
        // Starts opening or closing
        isAnimating = true;

        timerOne.setTimeout(function() {
            // Finished opening or closing
            isAnimating = false;
            // Set fake loading
            isLoading = true;
        }, transitionDuration);

        timerTwo.setTimeout(function() {
            // Finish fake loading
            isLoading = false;
        }, transitionDuration * 2);
    }

    Behavior on opacity {
        NumberAnimation {
            target: root
            property: "opacity"
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
    }

    // TODO: Remove debugging helpers
    // ColumnLayout {
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     anchors.top: parent.top

    //     Text {
    //         text: "isOpen: " + root.isOpen
    //     }
    //     Text {
    //         text: "isAnimating: " + root.isAnimating
    //     }
    //     Text {
    //         text: "State: " + root.state
    //     }

    //     z: 2
    // }

    // TODO: Move timer to a utils component
    Timer {
        id: timerOne

        function setTimeout(callback, timeoutDuration) {
            timerOne.interval = timeoutDuration;
            timerOne.repeat = false;
            timerOne.triggered.connect(callback);
            timerOne.triggered.connect(function release() {
                timerOne.triggered.disconnect(callback);
                timerOne.triggered.disconnect(release);
            });
            timerOne.start();
        }
    }
    // TODO: Remove timer for used for fake loading
    Timer {
        id: timerTwo

        function setTimeout(callback, timeoutDuration) {
            timerTwo.interval = timeoutDuration;
            timerTwo.repeat = false;
            timerTwo.triggered.connect(callback);
            timerTwo.triggered.connect(function release() {
                timerTwo.triggered.disconnect(callback);
                timerTwo.triggered.disconnect(release);
            });
            timerTwo.start();
        }
    }

    // Restart button
    VPNIconButton {
        id: connectionInfoRestartButton

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: VPNTheme.theme.windowMargin / 2
        anchors.rightMargin: VPNTheme.theme.windowMargin / 2

        accessibleName: "Restart speed test"
        buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
        enabled: root.state === "open-loading"
        z: 1

        Image {
            anchors.centerIn: connectionInfoRestartButton
            source: "qrc:/nebula/resources/refresh.svg"
            sourceSize.height: VPNTheme.theme.iconSize
            sourceSize.width: VPNTheme.theme.iconSize
        }
    }

    // Content
    VPNConnectionInfoContent {
        visible: root.state === "open-ready"
    }

    // Loader
    Text {
        text: "Loading …"
        visible: root.state === "open-loading"
    }

    Item {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        height: parent.width / 2
        visible: root.state === "open-loading"
        width: height
        onVisibleChanged: {
            if (visible) {
                loadingAnimation.play();
            } else {
                loadingAnimation.stop();
            }
        }

        VPNLottieAnimation {
            id: loadingAnimation
            source: ":/nebula/resources/animations/lock_animation.json"
        }
    }

}