/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1
import themes 0.1

CheckBox {
    // TODO
    // property var accessibleName

    id: checkBox

    property var uiState: Theme.uiState

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
                target: checkmarkBg

                color: !!checkBox.checked ? Theme.blue : Theme.bgColor
            }

        },
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked ? Theme.bluePressed : Theme.fontColorDark
            }

            PropertyChanges {
                target: checkmarkBg

                color: checkBox.checked ? Theme.bluePressed : Theme.greyPressed
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked || checkBox.activeFocus ? Theme.blueHovered : Theme.fontColorDark
            }

            PropertyChanges {
                target: checkmarkBg

                color: checkBox.checked ? Theme.blueHovered : "#DBDBDB"
            }

        }
    ]

    Item {
        id: checkmark

        height: 20
        width: 20
        anchors.fill: checkBoxIndicator

        Rectangle {
            id: checkmarkBg

            color: checkBox.checked ? Theme.blue : Theme.bgColor
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

            source: "qrc:/nebula/resources/checkmark.svg"
            sourceSize.height: 13
            sourceSize.width: 12
            visible: false
            anchors.centerIn: checkmark
        }

        VPNOpacityMask {
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
        border.color: checkBox.checked ? Theme.blue : Theme.fontColor
        border.width: 2
        radius: 4
        antialiasing: true
        state: checkBox.state

        VPNFocusOutline {
            visible: true
            anchors.margins: -3
            focusedComponent: checkBox
            focusColorScheme: Theme.blueButton
        }

        Behavior on border.color {
            PropertyAnimation {
                duration: 100
            }

        }

    }

}
