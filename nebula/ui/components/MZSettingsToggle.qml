/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5

import Mozilla.Shared 1.0
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

CheckBox {
    id: vpnSettingsToggle

    property var toggleColor: MZTheme.theme.vpnToggleConnected
    property var uiState: MZTheme.theme.uiState
    property alias forceFocus: vpnSettingsToggle.focus
    property var toolTipTitle
    property string accessibleName

    onClicked: toolTip.hide()
    onActiveFocusChanged: if(activeFocus) MZUiUtils.scrollToComponent(vpnSettingsToggle)

    // Workaround for https://bugreports.qt.io/browse/QTBUG-101026
    // Prevents 'TypeError: Property 'styleFont' of object MZUIStates_QMLTYPE_14(0x600002d582a0) is not a function' 
    font.pixelSize: undefined

    implicitHeight: MZTheme.theme.toggleHeight
    implicitWidth: MZTheme.theme.toggleWidth
    states: [
        State {
            when: checked

            PropertyChanges {
                target: vpnSettingsToggle
                toggleColor: MZTheme.theme.vpnToggleConnected
            }

            PropertyChanges {
                target: cursor
                x: 24
            }

        },

        State {
            when: !checked

            PropertyChanges {
                target: vpnSettingsToggle
                toggleColor: MZTheme.theme.vpnToggleDisconnected

            }

            PropertyChanges {
                target: cursor
                x: 3
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                target: cursor
                property: "x"
                duration: 200
            }
        }
    ]

    MZFocusBorder {
        id: focusHandler

        anchors.fill: hoverPressHandler
        border.color: toggleColor.focusBorder
        color: MZTheme.theme.transparent
        anchors.margins: -1
        radius: 50
        opacity: vpnSettingsToggle.activeFocus ? 1: 0
    }

    Rectangle {
        id: cursor

        height: 18
        width: 18
        radius: 9
        color: MZTheme.theme.white
        z: 1
        anchors.verticalCenter: vpnSettingsToggle.verticalCenter
    }

    MZMouseArea {
        id: mouseArea

        anchors.fill: hoverPressHandler
        targetEl: uiPlaceholder
        hoverEnabled: parent.enabled
    }

    Rectangle {
        id: hoverPressHandler

        color: "#C2C2C2"
        opacity: 0
        z: -1
        anchors.fill: uiPlaceholder
        radius: height / 2
        anchors.margins: -4
        state: uiPlaceholder.state
        states: [
            State {
                name: uiState.stateHovered
                PropertyChanges {
                    target: hoverPressHandler
                    opacity: parent.enabled ? 0.2 : 0;

                }
            },
            State {
                name: uiState.statePressed
                PropertyChanges {
                    target: hoverPressHandler
                    opacity: parent.enabled ? .3 : 0
                }
            }
        ]

        transitions: [
            Transition {
                PropertyAnimation {
                    target: hoverPressHandler
                    property: "opacity"
                    duration: 200
                }
            }

        ]
    }

    focusPolicy: Qt.StrongFocus

    Keys.onReleased: event => {
        if (event.key === Qt.Key_Return)
            uiPlaceholder.state = uiState.stateDefault;
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            uiPlaceholder.state = uiState.statePressed;
    }

    Keys.onReturnPressed: clicked()
    Keys.onSpacePressed: clicked()
    Accessible.onPressAction: clicked()
    Accessible.onToggleAction: clicked()
    Accessible.focusable: true
    Accessible.name: accessibleName
    Accessible.ignored: !visible

    Rectangle {
        id: uiPlaceholder /* Binding loop hack-around */
        height: 24
        width: 45
        color: MZTheme.theme.transparent
    }

    indicator:  MZUIStates {
        id: toggleBackground
        itemToFocus: vpnSettingsToggle
        itemToAnchor: uiPlaceholder
        colorScheme: toggleColor
        radius: height / 2
        showFocusRings: false
        startingState: checked ? MZTheme.theme.vpnToggleConnected.defaultColor : MZTheme.theme.vpnToggleDisconnected.defaultColor
    }
}
