/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNButtonBase {
    id: root

    property string labelText
    property variant fontName: VPNTheme.theme.fontInterFamily
    property var baseColor: VPNTheme.theme.linkButton
    property var linkColor: VPNTheme.theme.blueButton
    property var fontSize: VPNTheme.theme.fontSize
    property real textAlignment: Text.AlignHCenter
    property Component iconComponent

    radius: VPNTheme.theme.cornerRadius

    Keys.onReleased: event => {
        if (loaderVisible) {
            return
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space) {
            state = uiState.stateDefault;
        }
    }

    Accessible.name: labelText
    opacity: enabled? 1 : VPNTheme.theme.opacityDisabled

    Component.onCompleted: {
        state = Qt.binding(() => (
            enabled ? uiState.stateDefault : uiState.stateDisabled
        ));
        buttonMouseArea.hoverEnabled = Qt.binding(() => (
            enabled && loaderVisible === false
        ));
    }

    states: [
        State {
            name: uiState.stateDisabled

            PropertyChanges {
                target: label
                color: root.linkColor.buttonDisabled
            }
        },
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
        id: buttonMouseArea
        hoverEnabled: loaderVisible === false && parent.enabled
    }

    background: Rectangle {
        id: backgroundRect
        color: VPNTheme.theme.transparent
    }

    contentItem: RowLayout {
        spacing: VPNTheme.theme.windowMargin / 2

        Loader {
            id: iconLoader
            Layout.alignment: Qt.AlignVCenter
            sourceComponent: iconComponent
        }

        Label {
            id: label

            text: labelText
            color: root.linkColor.defaultColor
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
}
