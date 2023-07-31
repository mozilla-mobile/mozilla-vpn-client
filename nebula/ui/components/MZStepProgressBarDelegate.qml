/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */import QtQuick 2.5
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

import compat 0.1

Column {
    id: delegate
    property string iconSource
    property alias labelText: label.text
    property alias labelWidth: label.width
    property string accessibleName: ""
    property int currentState: MZStepProgressBarDelegate.State.Incomplete

    signal clicked

    enum State {
        Incomplete,
        Active,
        Complete
    }

    //Width is set to the size of the button, so the label exceeds bounds
    width: button.implicitWidth

    spacing: 8

    MZButtonBase {
        id: button

        anchors.horizontalCenter: parent.horizontalCenter

        implicitWidth: MZTheme.theme.progressBarDelegateHeightWidth
        implicitHeight: MZTheme.theme.progressBarDelegateHeightWidth

        enabled: currentState !== MZStepProgressBarDelegate.State.Incomplete
        radius: implicitWidth / 2

        onEnabledChanged: {
            if(!enabled) state = MZTheme.theme.uiState.stateDisabled
            else state = MZTheme.theme.uiState.stateDefault
        }

        Accessible.name: delegate.accessibleName

        onClicked: delegate.clicked()

        Rectangle {
            id: background

            anchors.centerIn: parent

            z: parent.z - 1
            radius: implicitWidth / 2
            color: MZTheme.colors.purple10

            implicitHeight: button.activeFocus || button.state === MZTheme.theme.uiState.stateHovered || button.state === MZTheme.theme.uiState.statePressed || delegate.currentState === MZStepProgressBarDelegate.State.Active ? parent.implicitHeight + 8 : 0
            implicitWidth: button.activeFocus || button.state === MZTheme.theme.uiState.stateHovered || button.state === MZTheme.theme.uiState.statePressed || delegate.currentState === MZStepProgressBarDelegate.State.Active ? parent.implicitWidth + 8 : 0

            Behavior on implicitHeight {
                enabled: button.state !== MZTheme.theme.uiState.stateHovered && button.state !== MZTheme.theme.uiState.statePressed

                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on implicitWidth {
                enabled: button.state !== MZTheme.theme.uiState.stateHovered && button.state !== MZTheme.theme.uiState.statePressed

                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Item {
            anchors.centerIn: parent

            height: MZTheme.theme.iconSize * 1.5
            width: MZTheme.theme.iconSize * 1.5

            MZIcon {
                id: icon

                source: delegate.currentState === MZStepProgressBarDelegate.State.Complete ? "qrc:/nebula/resources/checkmark-green.svg" : delegate.iconSource
            }

            MZColorOverlay {
                anchors.fill: parent

                color: delegate.currentState === MZStepProgressBarDelegate.State.Complete ? MZTheme.theme.green : MZTheme.theme.white
                source: icon
            }
        }

        MZUIStates {
            setMargins: -4
            radius: parent.radius
            colorScheme: MZTheme.theme.purpleStepProgressBarDelegate
            startingState: delegate.currentState === MZStepProgressBarDelegate.State.Active ? colorScheme.defaultColor : colorScheme.buttonDisabled
            showFocusRings: false
        }

        MZMouseArea {
            id: mouseArea
            hoverEnabled: parent.enabled
        }
    }

    MZInterLabel {
        id: label

        anchors.horizontalCenter: parent.horizontalCenter

        elide: Text.ElideRight
        maximumLineCount: 2
        font.pixelSize: MZTheme.theme.fontSizeSmallest
        lineHeightMode: Text.FixedHeight
        lineHeight: MZTheme.theme.controllerInterLineHeight
        color: delegate.currentState !== MZStepProgressBarDelegate.State.Incomplete ? MZTheme.colors.purple70 : MZTheme.theme.fontColorDark
    }
}
