/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
    property bool isLoading: false
    property bool isOpen: false
    property bool isTransitioning: false
    property int transitionDuration: 750

    id: root

    clip: true
    color: VPNTheme.colors.primary
    opacity: 0
    state: "closed"
    states: [
        State {
            name: "closed"
            when: !isOpen && !isTransitioning

            PropertyChanges {
                target: root
                opacity: 0
                visible: false
            }
        },
        State {
            name: "opening"
            when: isOpen && isTransitioning

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        },
        State {
            name: "closing"
            when: !isOpen && isTransitioning

            PropertyChanges {
                target: root
                opacity: 0
                visible: true
            }
        },
        State {
            name: "open-loading"
            when: isOpen && !isTransitioning && isLoading

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        },
        State {
            name: "open-ready"
            when: isOpen && !isTransitioning && !isLoading

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        }
    ]
    width: parent.width

    onIsOpenChanged: () => {
        // Start opening/closing transition
        isTransitioning = true;

        timerOne.setTimeout(function() {
            // Finished opening/closing transition
            isTransitioning = false;
            // Set fake loading state: Starting connection speedtest
            isLoading = true;
        }, transitionDuration);

        timerTwo.setTimeout(function() {
            // Set fake loading state: Finished connection speedtest
            isLoading = false;
        }, transitionDuration * 10);
    }

    Behavior on opacity {
        NumberAnimation {
            target: root
            property: "opacity"
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
    }

    VPNConnectionInfoContent {
        id: connectionInfoContent

        opacity: visible && root.state !== "closing" ? 1 : 0
        visible: root.state === "open-ready" || root.state === "closing"

        VPNIconButton {
            id: connectionInfoRestartButton

            anchors {
                top: parent.top
                right: parent.right
                topMargin: VPNTheme.theme.windowMargin / 2
                rightMargin: VPNTheme.theme.windowMargin / 2
            }
            // TODO: Replace with localized string
            accessibleName: "Restart speed test"
            buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
            enabled: connectionInfoContent.visible && !root.isLoading
            z: 1

            onClicked: {
                // Set fake loading state: Restart connection speedtest
                root.isLoading = true;

                timerThree.setTimeout(function() {
                    // Set fake loading state: Finished connection speedtest
                    root.isLoading = false;
                }, transitionDuration * 10);
            }

            Image {
                anchors.centerIn: connectionInfoRestartButton
                opacity: 0.8
                source: "qrc:/nebula/resources/refresh.svg"
                sourceSize.height: VPNTheme.theme.iconSize * 1.5
                sourceSize.width: VPNTheme.theme.iconSize * 1.5
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: root.state !== "closing"
                    ? root.transitionDuration
                    : root.transitionDuration * 2
            }
        }
    }

    VPNLogoLoader {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: -1 * VPNTheme.theme.rowHeight
        }
        height: VPNTheme.theme.desktopAppWidth * 0.33
        showLoader: root.state === "open-loading"
        width: height

        VPNMetropolisLabel {
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.bottom
            }
            color: VPNTheme.colors.white
            font.pixelSize: VPNTheme.theme.fontSizeLarge
            // TODO: Replace with localized string
            text: "Testing speed …"
        }
    }

    // TODO: Remove timers that are used for setting fake loading
    VPNTimer {
        id: timerOne
    }
    VPNTimer {
        id: timerTwo
    }
    VPNTimer {
        id: timerThree
    }

}
