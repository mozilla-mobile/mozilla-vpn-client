/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5

import Mozilla.Shared 1.0
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

RoundButton {
    id: root

    property var visualStateItem: root
    property var uiState: MZTheme.theme.uiState
    property bool loaderVisible: false
    property var handleKeyClick: function() { clicked() }

    focusPolicy: Qt.StrongFocus
    Keys.onPressed: event => {
        if (loaderVisible) {
            return;
        }

        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space)
            visualStateItem.state = uiState.statePressed;

    }
    Keys.onReleased: event => {
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
    Accessible.onPressAction: enabled ? handleKeyClick() : function() { }
    Accessible.focusable: enabled
    Accessible.ignored: !visible
    // In Qt 6.2.4, Windows Accessibility's focusable property is mapped to activeFocusOnTab instead of Accessible.focusable or enabled.
    // Use activeFocusOnTab as a workaround. The issue has been fixed in Qt 6.5.1. (See QAccessibleQuickItem::state)
    activeFocusOnTab: enabled

    onActiveFocusChanged: {
        if (!activeFocus) {
            return visualStateItem.state = uiState.stateDefault;
        }
        visualStateItem.state = uiState.stateFocused;

        MZUiUtils.scrollToComponent(root)
    }

    background: Rectangle {
        color: MZTheme.theme.transparent
    }

}
