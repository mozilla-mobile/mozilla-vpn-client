/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
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
            when: (VPNConnectionBenchmark.state !== VPNConnectionBenchmark.StateInitial
                && VPNConnectionBenchmark.state !== VPNConnectionBenchmark.StateReady)
                && isOpen
                && !isTransitioning

            PropertyChanges {
                target: root
                opacity: 1
                visible: true
            }
        },
        State {
            name: "open-ready"
            when: VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateReady
                && isOpen
                && !isTransitioning

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

        if (VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateInitial) {
            VPNConnectionBenchmark.start();
        } else if (VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateRunning
            || VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateReady) {
            VPNConnectionBenchmark.stop();
        }

        timer.setTimeout(function() {
            // Finished opening/closing transition
            isTransitioning = false;
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
            enabled: connectionInfoContent.visible
            z: 1

            onClicked: {
                VPNConnectionBenchmark.start();
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
            text: VPNl18n.ConnectionInfoLoadingIndicatorLabel
        }
    }

    VPNTimer {
        id: timer
    }

}
