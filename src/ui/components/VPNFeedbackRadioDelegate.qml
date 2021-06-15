/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme

RadioDelegate {
    property var value
    property alias iconSource: img.source
    property var heightWidth: 30
    property var uiState: Theme.uiState
    id: radio
    checked: false
    implicitHeight: heightWidth
    implicitWidth: heightWidth
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

    ColorOverlay {
        id: colorOverlay
        anchors.fill: img
        source: img
        color: radio.checked || radio.activeFocus ? Theme.blue : Theme.fontColor
        Behavior on color {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            mouseArea.changeState(uiState.stateDefault);
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            radio.clicked();
    }


    background: VPNInputBackground {
        id: bg
        radius: 22
        border.color: Theme.input.focusBorder
        color: "transparent"
        border.width: 2
        opacity: radio.focus ? 1 : 0
        z: 1

        VPNFocusOutline {
            color: "transparent"
            opacity: radio.checked && !radio.focus ? 1 : 0
            border.color: Theme.blueFocusOutline
            border.width: 3
        }

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
                color: Theme.bluePressed
            }
            PropertyChanges {
                target: bg
                border.color: Theme.bluePressed
            }
        },
        State {
            name: uiState.stateDefault

            PropertyChanges {
                target: colorOverlay
                color: radio.checked ? Theme.blue : Theme.fontColor
            }
        },
        State {
            name: uiState.stateHovered
            PropertyChanges {
                target: colorOverlay
                color: Theme.fontColorDark
            }
        }
    ]

}
