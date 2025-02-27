/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0

Rectangle {
    id: root

    property var uiState:MZTheme.theme.uiState
    property variant itemToFocus: parent
    property variant itemToAnchor: parent
    property var borderWidth: MZTheme.theme.focusBorderWidth
    property var colorScheme: MZTheme.colors.linkButton
    property real setMargins: -2
    property bool showFocusRings: true
    property var startingState: colorScheme.defaultColor

    signal clicked()

    anchors.fill: itemToAnchor
    antialiasing: true
    color: MZTheme.colors.transparent
    radius: MZTheme.theme.cornerRadius
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
                color: colorScheme.buttonDisabled ? colorScheme.buttonDisabled : colorScheme.defaultColor
                opacity: 1
            }

        },
        State {
            when: itemToAnchor.state === uiState.stateFocused

            PropertyChanges {
                target: buttonBackground
                color: colorScheme.defaultColor
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

    MZFocusOutline {
        visible: showFocusRings
        anchors.margins: setMargins
        focusedComponent: itemToFocus
        focusColorScheme: colorScheme
    }

    Rectangle {
        id: buttonBackground

        anchors.fill: parent
        radius: parent.radius
        antialiasing: true

        //Intentionally not binding color to starting state color to avoid unecessary animation
        color: {
            buttonBackground.color = root.startingState
        }
    }

    MZFocusBorder {
        id: focusInnerBorder

        visible: showFocusRings
        color: buttonBackground.color
        border.width: borderWidth
    }

}
