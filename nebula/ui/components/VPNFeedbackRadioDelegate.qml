/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import compat 0.1

RadioDelegate {
    property var value
    property alias iconSource: img.source
    property var heightWidth: 30
    property var uiState: VPNTheme.theme.uiState
    id: radio
    checked: false
    implicitHeight: VPNTheme.theme.rowHeight
    implicitWidth: VPNTheme.theme.rowHeight
    activeFocusOnTab: true
    Component.onCompleted: {
        state = uiState.stateDefault
    }

    indicator: VPNIcon {
        id: img
        anchors.centerIn: parent
        antialiasing: true
        sourceSize.height: heightWidth
        sourceSize.width: heightWidth
    }

    VPNColorOverlay {
        id: colorOverlay
        anchors.fill: img
        source: img
        color: radio.checked || radio.focus || radio.activeFocus ? VPNTheme.theme.blue : VPNTheme.theme.fontColor
        Behavior on color {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.stateDefault);
    }

    Keys.onReleased: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            radio.clicked();
    }


    background: VPNFocusOutline {
            opacity: radio.checked || radio.activeFocus ? 1 : 0
            color: "transparent"
            border.width: 4
            border.color: VPNTheme.theme.blueFocusOutline
            anchors.margins: 2
            radius: height

            Behavior on opacity {
                PropertyAnimation {
                    duration: 100
                }
            }
        }

    VPNMouseArea {
        id: mouseArea
        onPressedChanged: if (pressed) radio.forceActiveFocus()
        onMouseAreaClicked: function() {
            radio.checked = !radio.checked;
        }
    }

    states: [
        State {
            name: uiState.statePressed
            PropertyChanges {
                target: colorOverlay
                color: VPNTheme.theme.bluePressed
            }
        },
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: colorOverlay
                color: radio.checked || radio.focus || radio.activeFocus ? VPNTheme.theme.blue : VPNTheme.theme.fontColor
            }
        },
        State {
            name: uiState.stateHovered
            PropertyChanges {
                target: colorOverlay
                color: VPNTheme.theme.blue
            }
        }
    ]

}
