/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

CheckBox {
    property var uiState: VPNTheme.theme.uiState
    property string accessibleName
    property string accessibleDescription

    id: checkBox

    signal clicked()

    Layout.preferredHeight: 20
    Layout.preferredWidth: 20
    Layout.margins: 2
    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    Component.onCompleted: state = uiState.stateDefault
    hoverEnabled: false
    onActiveFocusChanged: {
        if (!activeFocus)
            mouseArea.changeState(uiState.stateDefault);

        if (activeFocus && typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible)
            vpnFlickable.ensureVisible(checkBox);

    }
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.statePressed);

    }
    Keys.onReleased: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.stateDefault);

    }
    Keys.onReturnPressed: checkBox.clicked()
    Keys.onSpacePressed: checkBox.clicked()

    Accessible.onPressAction: clicked()
    Accessible.onToggleAction: clicked()
    Accessible.focusable: true
    Accessible.name: accessibleName
    Accessible.description: accessibleDescription

    states: [
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked || checkBox.activeFocus ? VPNTheme.theme.blue : VPNTheme.theme.fontColor
            }

        },
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked ? VPNTheme.theme.bluePressed : VPNTheme.theme.fontColorDark
            }
        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked || checkBox.activeFocus ? VPNTheme.theme.blueHovered : VPNTheme.theme.fontColorDark
            }

        }
    ]

    VPNCheckmark {
        anchors.fill: checkBoxIndicator
    }

    VPNMouseArea {
        id: mouseArea
    }

    indicator: Rectangle {
        id: checkBoxIndicator

        height: 20
        width: 20
        color: VPNTheme.theme.bgColor
        border.color: checkBox.checked ? VPNTheme.theme.blue : VPNTheme.theme.fontColor
        border.width: 2
        radius: 4
        antialiasing: true
        state: checkBox.state

        VPNFocusOutline {
            visible: true
            anchors.margins: -3
            focusedComponent: checkBox
            focusColorScheme: VPNTheme.theme.blueButton
        }

        Behavior on border.color {
            PropertyAnimation {
                duration: 100
            }

        }

    }

}
