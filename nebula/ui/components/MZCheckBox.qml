/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import utils 0.1

CheckBox {
    property var uiState: MZTheme.theme.uiState
    property string accessibleName

    id: checkBox

    signal clicked()

    Layout.preferredHeight: MZTheme.theme.checkBoxHeightWidth
    Layout.preferredWidth: MZTheme.theme.checkBoxHeightWidth
    Layout.margins: 2
    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    Component.onCompleted: state = uiState.stateDefault
    hoverEnabled: false
    onActiveFocusChanged: {
        if (!activeFocus)
            return mouseArea.changeState(uiState.stateDefault);

        MZUiUtils.scrollToComponent(checkBox)
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
    Accessible.role: Accessible.CheckBox

    states: [
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked || checkBox.activeFocus ? MZTheme.theme.blue : MZTheme.theme.fontColor
            }

        },
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked ? MZTheme.theme.bluePressed : MZTheme.theme.fontColorDark
            }
        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked || checkBox.activeFocus ? MZTheme.theme.blueHovered : MZTheme.theme.fontColorDark
            }

        }
    ]

    MZIcon {
        id: checkmarkIcon
        source: "qrc:/nebula/resources/checkmark-blue50.svg"
        sourceSize.width: MZTheme.theme.checkmarkHeightWidth
        sourceSize.height: MZTheme.theme.checkmarkHeightWidth
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 1
        opacity: checkBox.checked ? 1 : 0
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    MZMouseArea {
        id: mouseArea
    }

    indicator: Rectangle {
        id: checkBoxIndicator

        anchors.fill: parent
        color: MZTheme.theme.bgColor
        border.color: checkBox.checked ? MZTheme.theme.blue : MZTheme.theme.fontColor
        border.width: 2
        radius: MZTheme.theme.cornerRadius
        antialiasing: true
        state: checkBox.state

        MZFocusOutline {
            visible: true
            anchors.margins: -3
            focusedComponent: checkBox
            focusColorScheme: MZTheme.theme.blueButton
        }

        Behavior on border.color {
            PropertyAnimation {
                duration: 100
            }

        }

    }

}
