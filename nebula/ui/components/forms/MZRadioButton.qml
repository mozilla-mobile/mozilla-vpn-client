/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

RadioDelegate {
    id: radioControl

    property bool isHoverable: true
    property var accessibleName: ""
    property var uiState: MZTheme.theme.uiState

    signal clicked()

    activeFocusOnTab: true

    Component.onCompleted: {
        state = uiState.stateDefault
    }

    onActiveFocusChanged: {
        if (!radioControl.activeFocus)
            return mouseArea.changeState(uiState.stateDefault);

        MZUiUtils.scrollToComponent(radioControl)
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
                color: radioControl.checked ? MZTheme.colors.bluePressed : MZTheme.colors.greyPressed
                scale: 0.55
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked? MZTheme.colors.bluePressed : MZTheme.colors.fontColorDark
            }

        },
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.colors.blue : MZTheme.colors.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked || radioControl.activeFocus ? MZTheme.colors.blue : MZTheme.colors.fontColor
            }

        },
        State {
            name: uiState.stateHovered

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? MZTheme.colors.bluePressed : MZTheme.colors.greyHovered
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? MZTheme.colors.bluePressed : MZTheme.colors.fontColor
            }

        }
    ]

    background: Rectangle {
        color: MZTheme.colors.transparent
    }

    MZMouseArea {
        id: mouseArea
    }

    indicator: Rectangle {
        id: radioButton

        anchors.left: parent.left
        implicitWidth: 20
        implicitHeight: 20
        radius: implicitWidth * 0.5
        border.width: MZTheme.theme.focusBorderWidth
        color: MZTheme.colors.bgColor
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
