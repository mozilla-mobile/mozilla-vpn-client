/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

RadioDelegate {
    id: radioControl

    property bool isHoverable: true
    property var accessibleName: ""
    property var uiState: VPNTheme.theme.uiState

    signal clicked()

    activeFocusOnTab: true

    Component.onCompleted: {
        state = uiState.stateDefault
    }

    onFocusChanged: {
        if (!radioControl.focus)
            return mouseArea.changeState(uiState.stateDefault);
        if (typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible)
            vpnFlickable.ensureVisible(radioControl);
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.stateDefault);
    }

    Keys.onReleased: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            radioControl.clicked();
    }

    Accessible.name: accessibleName
    Accessible.onPressAction: clicked()
    Accessible.focusable: true

    states: [
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? VPNTheme.theme.bluePressed : VPNTheme.theme.greyPressed
                scale: 0.55
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked? VPNTheme.theme.bluePressed : VPNTheme.theme.fontColorDark
            }

        },
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? VPNTheme.theme.blue : VPNTheme.theme.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked || radioControl.activeFocus ? VPNTheme.theme.blue : VPNTheme.theme.fontColor
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? VPNTheme.theme.bluePressed : VPNTheme.theme.greyHovered
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? VPNTheme.theme.bluePressed : VPNTheme.theme.fontColor
            }

        }
    ]

    background: Rectangle {
        color: VPNTheme.theme.transparent
    }

    VPNMouseArea {
        id: mouseArea
    }

    indicator: Rectangle {
        id: radioButton

        anchors.left: parent.left
        implicitWidth: 20
        implicitHeight: 20
        radius: implicitWidth * 0.5
        border.width: VPNTheme.theme.focusBorderWidth
        color: VPNTheme.theme.bgColor
        antialiasing: true
        smooth: true

        Rectangle {
            id: radioButtonInsetCircle
            anchors.fill: parent
            radius: radioButton.implicitHeight / 2

            Behavior on color {
                ColorAnimation {
                    duration: 200
                }

            }

        }

        // Radio focus outline
        VPNFocusOutline {
            focusedComponent: radioControl
            setMargins: -3
            radius: height / 2
        }

        Behavior on border.color {
            ColorAnimation {
                duration: 200
            }

        }

    }

}
