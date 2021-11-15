/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

RadioDelegate {
    id: radioControl

    property bool isHoverable: true
    property var radioButtonLabelText
    property var accessibleName
    property var uiState: Theme.uiState

    signal clicked()

    ButtonGroup.group: radioButtonGroup
    width: parent.width
    height: Theme.rowHeight

    Component.onCompleted: {
        state = uiState.stateDefault
    }

    onFocusChanged: {
        if (!radioControl.focus)
            return mouseArea.changeState(uiState.stateDefault);
        if (typeof (ensureVisible) !== "undefined")
            ensureVisible(radioControl);
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.stateDefault);
    }

    Keys.onReleased: {
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
                color: radioControl.checked ? Theme.bluePressed : "#C2C2C2"
                scale: 0.55
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked? Theme.bluePressed : "#3D3D3D"
            }

        },
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.blue : Theme.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked || radioControl.activeFocus ? Theme.blue : Theme.fontColor
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.bluePressed : Theme.greyHovered
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? Theme.bluePressed : Theme.fontColor
            }

        }
    ]

    VPNRadioButtonLabel {
        id: radioButtonLabel

        text: radioButtonLabelText
    }

    background: Rectangle {
        color: "transparent"
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
        border.width: Theme.focusBorderWidth
        color: Theme.bgColor
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
