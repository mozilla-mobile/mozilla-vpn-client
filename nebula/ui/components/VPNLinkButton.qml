/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import themes 0.1

VPNButtonBase {
    id: root

    property var labelText
    property variant fontName: Theme.fontInterFamily
    property var baseColor: Theme.linkButton
    property var linkColor: Theme.blueButton
    property var fontSize: Theme.fontSize
    property var textAlignment: Text.AlignHCenter
    property var buttonPadding: Theme.hSpacing


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
        color: "transparent"
    }

    contentItem: Label {
        id: label

        text: labelText
        horizontalAlignment: textAlignment
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: fontSize
        font.family: fontName
        wrapMode: Text.WordWrap
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
    }
}
