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
    property var iconSize: 30
    property var uiState: VPNTheme.theme.uiState

    id: radio

    implicitHeight: VPNTheme.theme.rowHeight
    implicitWidth: VPNTheme.theme.rowHeight
    activeFocusOnTab: true
    focus: true

    Component.onCompleted: {
        state = uiState.stateDefault
    }

    indicator: Item {
        anchors.centerIn: parent
        height: iconSize
        width: iconSize

        VPNIcon {
            id: img
            antialiasing: true
            sourceSize.height: parent.height
            sourceSize.width: parent.width
        }

        VPNColorOverlay {
            anchors.fill: parent
            source: img
            color: VPNTheme.colors.blue
            visible: radio.checked || radio.state === uiState.stateHovered
                || radio.state === uiState.statePressed
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
        anchors.margins: VPNTheme.theme.focusBorderWidth

        border.color: VPNTheme.theme.blueFocusOutline
        border.width: VPNTheme.theme.focusBorderWidth * 2
        color: VPNTheme.theme.transparent
        opacity: radio.checked || radio.activeFocus ? 1 : 0
        radius: height

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    VPNMouseArea {
        id: mouseArea
        onMouseAreaClicked: function() {
            radio.forceActiveFocus();
            radio.checked = !radio.checked;
        }
    }
}
