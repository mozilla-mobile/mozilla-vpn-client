/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.14

import themes 0.1

RoundButton {
    id: root

    property var visualStateItem: root
    property var uiState: Theme.uiState
    property var loaderVisible: false
    property var handleKeyClick: function() { clicked() }

    focusPolicy: Qt.StrongFocus
    Keys.onPressed: {
        if (loaderVisible) {
            return;
        }

        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            visualStateItem.state = uiState.statePressed;

    }
    Keys.onReleased: {
        if (loaderVisible) {
            return;
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space) {
            visualStateItem.state = uiState.stateDefault;
        }
        if (event.key === Qt.Key_Return)
            handleKeyClick();
    }

    onClicked: {
        if (typeof(toolTip) !== "undefined" && toolTip.opened) {
            toolTip.close();
        }
    }

    Accessible.role: Accessible.Button
    Accessible.onPressAction: handleKeyClick()
    Accessible.focusable: true

    onActiveFocusChanged: {
        if (!activeFocus)
            return visualStateItem.state = uiState.stateDefault;

        if (typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible)
            return vpnFlickable.ensureVisible(visualStateItem);
    }

    background: Rectangle {
        color: "transparent"
    }

}
