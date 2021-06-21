/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

CheckBox {
    // TODO
    // property var accessibleName

    id: checkBox

    property var uiState: Theme.uiState

    signal clicked()

    height: 20
    width: 20
    Layout.alignment: Qt.AlignTop
    Component.onCompleted: state = uiState.stateDefault
    hoverEnabled: false
    onActiveFocusChanged: {
        if (!activeFocus)
            mouseArea.changeState(uiState.stateDefault);

        if (activeFocus && typeof (ensureVisible) !== "undefined")
            ensureVisible(checkBox);

    }
    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.statePressed);

    }
    Keys.onReleased: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.stateDefault);

    }
    Keys.onReturnPressed: checkBox.clicked()
    // TODO
    // Accessible.name: accessibleName
    Accessible.onPressAction: clicked()
    Accessible.onToggleAction: clicked()
    Accessible.focusable: true
    states: [
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked || checkBox.activeFocus ? Theme.blue : Theme.fontColor
            }

            PropertyChanges {
                target: checkmark
                opacity: checkBox.checked ? 1 : 0
                checkmarkColor: checkBox.checked ? Theme.blue : "#DBDBDB"
            }

        },
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked ? Theme.bluePressed : Theme.fontColorDark
            }

            PropertyChanges {
                target: checkmark
                opacity: 1
                checkmarkColor: checkBox.checked ? Theme.bluePressed : Theme.greyPressed
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked ? Theme.blueHovered : Theme.fontColorDark
            }

            PropertyChanges {
                target: checkmark
                opacity: 1
                checkmarkColor: checkBox.checked ? Theme.blueHovered : "#DBDBDB"
            }

        }
    ]

    Item {
        id: checkmark

        property var checkmarkColor: "#DBDBDB"

        height: 20
        width: 20
        anchors.fill: checkBoxIndicator

        Rectangle {
            id: checkmarkBg

            color: checkmark.checkmarkColor
            height: 20
            width: 20
            antialiasing: true
            smooth: true
            visible: false

            Behavior on color {
                PropertyAnimation {
                    duration: 200
                }

            }

        }

        Image {
            id: checkmarkIcon

            source: "../resources/checkmark.svg"
            sourceSize.height: 13
            sourceSize.width: 12
            visible: false
            anchors.centerIn: checkmark
        }

        OpacityMask {
            anchors.centerIn: checkmark
            height: checkmarkIcon.height
            width: checkmarkIcon.width
            source: checkmarkBg
            maskSource: checkmarkIcon
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }

        }

    }

    VPNMouseArea {
        id: mouseArea
    }

    indicator: Rectangle {
        id: checkBoxIndicator

        height: 20
        width: 20
        color: Theme.bgColor
        border.color: Theme.fontColor
        border.width: 2
        radius: 4
        antialiasing: true
        state: checkBox.state

        VPNUIStates {
            itemToFocus: checkBox
            colorScheme: Theme.blueButton
            visible: isEnabled
        }

        Behavior on border.color {
            PropertyAnimation {
                duration: 100
            }

        }

    }

}
