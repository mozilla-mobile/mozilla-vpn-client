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
    property string accessibleName: ""
    property bool isRadioButtonLabelAccessible: true
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

    function handleAccessiblePressAction() {
        let prevAccessibleName = radioControl.Accessible.name;

        clicked();

        // Currently, Qt doesn't have built-in accessibility support for screen readers to announce the selection of a radio button through
        // PressAction. A workaround on Windows is to manually raise the "Selected" state as an Accessible Notification.
        // Don't do this if the selection also changes focus, as the screen reader will read the newly focused control. Also, don't do this
        // if the selection changes the Accessible Name, as the screen reader will read the new name and state, making the extra "Selected"
        // notification redundant.
        if (radioControl.checked && !radioControl.Accessible.ignored 
                && radioControl.activeFocus && radioControl.Accessible.name === prevAccessibleName)
            MZAccessibleNotification.notify(radioControl, MZI18n.AccessibilitySelected);
    }

    Accessible.name: accessibleName
    Accessible.onPressAction: handleAccessiblePressAction()
    Accessible.focusable: true
    Accessible.ignored: !visible

    states: [
        State {
            name: uiState.statePressed

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.colors.normalButton.buttonPressed : MZTheme.colors.radioButtonDisabledPressed
                scale: 0.55
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? MZTheme.colors.normalButton.buttonPressed : MZTheme.colors.fontColorDark
            }

        },
        State {
            name: uiState.stateDefault
            when: radioButton.enabled

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.colors.normalButton.defaultColor : MZTheme.colors.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked || radioControl.activeFocus ? MZTheme.colors.normalButton.defaultColor : MZTheme.colors.fontColor
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.colors.normalButton.buttonPressed : MZTheme.colors.radioButtonDisabledHovered
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? MZTheme.colors.normalButton.buttonPressed : MZTheme.colors.fontColor
            }

        },
        State {
            name: uiState.stateDisabled
            when: !radioControl.enabled

            PropertyChanges {
                target: radioButtonInsetCircle
                color: MZTheme.colors.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: MZTheme.colors.disabledButtonColor
            }

        }
    ]

    MZRadioButtonLabel {
        id: radioButtonLabel
        Accessible.ignored: !isRadioButtonLabelAccessible || !visible
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
        color: MZTheme.colors.transparent
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
        color: MZTheme.colors.bgColor
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
