/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

Rectangle {
    id: root

    property var uiState:VPNTheme.theme.uiState
    property variant itemToFocus: parent
    property variant itemToAnchor: parent
    property var borderWidth: VPNTheme.theme.focusBorderWidth
    property var colorScheme: VPNTheme.theme.linkButton
    property var setMargins: -2
    property var showFocusRings: true

    signal clicked()

    anchors.fill: itemToAnchor
    antialiasing: true
    color: "transparent"
    radius: VPNTheme.theme.cornerRadius
    z: -1
    states: [
        State {
            when: itemToAnchor.state === uiState.stateDefault

            PropertyChanges {
                target: buttonBackground
                color: colorScheme.defaultColor
            }

        },
        State {
            when: itemToAnchor.state === uiState.stateHovered

            PropertyChanges {
                target: buttonBackground
                color: colorScheme.buttonHovered
                opacity: 1
            }

        },
        State {
            when: itemToAnchor.state === uiState.statePressed

            PropertyChanges {
                target: buttonBackground
                color: colorScheme.buttonPressed
                opacity: 1
            }

        },
        State {
            when: itemToAnchor.state === uiState.stateDisabled

            PropertyChanges {
                target: buttonBackground
                color: colorScheme.buttonDisabled
                opacity: 1
            }

        }
    ]
    transitions: [
        Transition {
            ColorAnimation {
                target: buttonBackground
                duration: 200
            }

            PropertyAnimation {
                target: buttonBackground
                property: "opacity"
                duration: 200
            }

        }
    ]

    VPNFocusOutline {
        visible: showFocusRings
        anchors.margins: setMargins
        focusedComponent: itemToFocus
        focusColorScheme: colorScheme
    }

    Rectangle {
        id: buttonBackground

        anchors.fill: parent
        radius: parent.radius
        color: colorScheme.defaultColor
        antialiasing: true
    }

    VPNFocusBorder {
        id: focusInnerBorder

        visible: showFocusRings
        color: buttonBackground.color
        border.width: borderWidth
    }

}
