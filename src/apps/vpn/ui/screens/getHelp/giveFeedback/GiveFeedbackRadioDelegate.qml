/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1
import compat 0.1

RadioDelegate {
    property var value
    property alias iconSource: img.source
    property real iconSize: 30
    property var uiState: MZTheme.theme.uiState
    readonly property bool isMobile: (Qt.platform.os === "android" || Qt.platform.os === "ios")

    id: radio

    implicitHeight: MZTheme.theme.rowHeight
    implicitWidth: MZTheme.theme.rowHeight
    activeFocusOnTab: true
    focus: true

    Component.onCompleted: {
        state = uiState.stateDefault
    }

    indicator: Item {
        anchors.centerIn: parent
        height: iconSize
        width: iconSize

        MZIcon {
            id: img
            antialiasing: true
            sourceSize.height: parent.height
            sourceSize.width: parent.width
        }

        MZColorOverlay {
            anchors.fill: parent
            source: img
            color: MZTheme.colors.blue
            //prevents iOS and Android from getting into a weird hover state
            visible: radio.checked || radio.state === uiState.statePressed
                     || (!radio.isMobile && radio.state === uiState.stateHovered)
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

    background: MZFocusOutline {
        anchors.margins: MZTheme.theme.focusBorderWidth

        border.color: MZTheme.theme.blueFocusOutline
        border.width: MZTheme.theme.focusBorderWidth * 2
        color: MZTheme.theme.transparent
        //OS Check to prevent multi-touch issue
        opacity: radio.checked || (!radio.isMobile && radio.activeFocus) ? 1 : 0
        radius: height

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    MZMouseArea {
        id: mouseArea
        onMouseAreaClicked: function() {
            radio.forceActiveFocus();
            radio.checked = true
        }
    }
}
