/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0

RadioDelegate {
    id: radioControl

    property bool isHoverable: true
    property var radioButtonLabelText
    property string accessibleName
    property var uiState: MZTheme.theme.uiState
    readonly property int labelX: radioButton.anchors.margins + radioButton.implicitWidth + radioButtonLabel.anchors.leftMargin

    signal clicked()

    ButtonGroup.group: radioButtonGroup
    implicitWidth: radioButton.implicitWidth + radioButtonLabel.implicitWidth + radioButtonLabel.anchors.leftMargin
    implicitHeight: Math.max(radioButtonLabel.implicitHeight, radioButton.implicitHeight)

    Component.onCompleted: {
        state = Qt.binding(() => radioControl.enabled ? uiState.stateDefault : uiState.stateDisabled)
    }

    onActiveFocusChanged: {
        if (!radioControl.focus)
            return mouseArea.changeState(uiState.stateDefault);
        if (typeof (ensureVisible) !== "undefined")
            ensureVisible(radioControl);
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
                color: radioControl.checked ? MZTheme.theme.bluePressed : MZTheme.theme.greyPressed
                scale: 0.55
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? MZTheme.theme.bluePressed : MZTheme.theme.fontColorDark
            }

        },
        State {
            name: uiState.stateDefault
            when: radioButton.enabled

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.theme.blue : MZTheme.theme.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked || radioControl.activeFocus ? MZTheme.theme.blue : MZTheme.theme.fontColor
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.theme.bluePressed : MZTheme.theme.greyHovered
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? MZTheme.theme.bluePressed : MZTheme.theme.fontColor
            }

        },
        State {
            name: uiState.stateDisabled
            when: !radioControl.enabled

            PropertyChanges {
                target: radioButtonInsetCircle
                color: MZTheme.theme.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: MZTheme.theme.greyButton.defaultColor
            }

        }
    ]

    MZRadioButtonLabel {
        id: radioButtonLabel

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: radioButton.right
        anchors.right: parent.right
        anchors.leftMargin: 16 + radioButton.anchors.rightMargin

        text: radioButtonLabelText
        lineHeightMode: Text.FixedHeight
        lineHeight: MZTheme.theme.labelLineHeight
        wrapMode: Text.Wrap
    }

    background: Rectangle {
        color: MZTheme.theme.transparent
    }

    MZMouseArea {
        id: mouseArea
        hoverEnabled: radioControl.isHoverable
        enabled: radioControl.enabled
    }

    indicator: Rectangle {
        id: radioButton

        objectName: "radioIndicator"
        anchors.margins: 2
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: 20
        implicitHeight: 20
        radius: implicitWidth * 0.5
        border.width: MZTheme.theme.focusBorderWidth
        color: MZTheme.theme.bgColor
        antialiasing: true
        smooth: true
        onActiveFocusChanged: if (activeFocus) parent.forceActiveFocus()

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
        MZFocusOutline {
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
