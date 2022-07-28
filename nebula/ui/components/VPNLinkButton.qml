/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNButtonBase {
    id: root

    property var labelText
    property variant fontName: VPNTheme.theme.fontInterFamily
    property var baseColor: VPNTheme.theme.linkButton
    property var linkColor: VPNTheme.theme.blueButton
    property var fontSize: VPNTheme.theme.fontSize
    property var textAlignment: Text.AlignHCenter
    property var buttonPadding: VPNTheme.theme.hSpacing


    radius: 4
    horizontalPadding: buttonPadding

    Keys.onReleased: event => {
        if (loaderVisible) {
            return
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space) {
            root.clicked();
            state = uiState.stateDefault;
        }
    }

    Accessible.name: labelText

    Component.onCompleted: state = uiState.stateDefault;

    states: [
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: label
                color: root.linkColor.buttonHovered
            }

        },
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: label
                color: root.linkColor.buttonPressed
            }

        },
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: label
                color: root.linkColor.defaultColor
            }

        },
        State {
            name: uiState.stateFocused

            PropertyChanges {
                target: label
                color: root.linkColor.defaultColor
            }

        }
    ]

    VPNUIStates {
        colorScheme: root.baseColor
        itemToFocus: root

        VPNFocusBorder {
            border.color: root.linkColor.defaultColor
            opacity: root.activeFocus ? 1 : 0
        }
    }

    VPNButtonLoader {
        id: loader
        state: loaderVisible ? "active" : "inactive"
    }

    VPNMouseArea {
        hoverEnabled: loaderVisible === false
    }

    background: Rectangle {
        id: backgroundRect
        color: VPNTheme.theme.transparent
    }

    contentItem: Label {
        id: label

        text: labelText
        horizontalAlignment: textAlignment
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: fontSize
        font.family: fontName
        wrapMode: Text.WordWrap
        opacity: loaderVisible ? 0 : 1
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
    }
}
