/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

Rectangle {
    property bool isOpen: false
    property bool isTransitioning: false
    property int transitionDuration: 750

    id: root

    clip: true
    color: MZTheme.colors.primary
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
            StateChangeScript {
                script: VPNConnectionBenchmark.reset();
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
                && VPNConnectionBenchmark.state !== VPNConnectionBenchmark.StateReady
                && VPNConnectionBenchmark.state !== VPNConnectionBenchmark.StateError)
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
        },
        State {
            name: "open-error"
            when: VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateError
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
        } else {
            VPNConnectionBenchmark.reset();
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

    ConnectionInfoContent {
        id: connectionInfoContent

        opacity: visible && root.state !== "closing" ? 1 : 0
        visible: root.state === "open-ready" || root.state === "closing"

        Behavior on opacity {
            NumberAnimation {
                duration: root.state !== "closing"
                    ? root.transitionDuration
                    : root.transitionDuration * 2
            }
        }
    }

    ConnectionInfoError {
        id: connectionInfoError
        objectName: "connectionInfoError"

        opacity: visible && root.state !== "closing" ? 1 : 0
        visible: root.state === "open-error" || root.state === "closing"
    }

    MZIconButton {
        id: connectionInfoRestartButton

        visible: connectionInfoContent.visible || connectionInfoError.visible

        anchors {
            top: parent.top
            right: parent.right
            topMargin: MZTheme.theme.windowMargin / 2
            rightMargin: MZTheme.theme.windowMargin / 2
        }
        // TODO: Replace with localized string
        accessibleName: "Restart speed test"
        buttonColorScheme: MZTheme.theme.iconButtonDarkBackground
        enabled: visible
        z: 1

        onClicked: {
            if (VPNConnectionBenchmark.state !== VPNConnectionBenchmark.StateRunning) {
                VPNConnectionBenchmark.start();
            }
        }

        Image {
            anchors.centerIn: connectionInfoRestartButton
            opacity: 0.8
            source: "qrc:/nebula/resources/refresh.svg"
            sourceSize.height: MZTheme.theme.iconSize * 1.5
            sourceSize.width: MZTheme.theme.iconSize * 1.5
        }
    }

    LogoLoader {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: -1 * MZTheme.theme.rowHeight
        }
        height: MZTheme.theme.desktopAppWidth * 0.33
        showLoader: root.state === "open-loading"
        width: height

        MZMetropolisLabel {
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.bottom
            }
            color: MZTheme.colors.white
            font.pixelSize: MZTheme.theme.fontSizeLarge
            text: MZI18n.ConnectionInfoLoadingIndicatorLabel
        }
    }

    MZTimer {
        id: timer
    }

    Component.onDestruction: VPNConnectionBenchmark.reset()

    Connections {
      target: MZNavigator

      function onCurrentComponentChanged() {
          // Stop connection speed test when navigating away from ScreenHome
          if (isOpen) {
            closeConnectionInfo();
          }
       }
    }
}
