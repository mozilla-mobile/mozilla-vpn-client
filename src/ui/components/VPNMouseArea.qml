/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import themes 0.1

MouseArea {
    id: mouseArea

    property var targetEl: parent
    property var uiState: Theme.uiState
    property var onMouseAreaClicked: function() { parent.clicked() }

    function changeState(stateName) {
        if (mouseArea.hoverEnabled)
           targetEl.state = stateName;
    }

    anchors.fill: parent
    hoverEnabled: true
    cursorShape: !hoverEnabled ? Qt.ForbiddenCursor : Qt.PointingHandCursor
    onEntered: changeState(uiState.stateHovered)
    onExited: changeState(uiState.stateDefault)
    onPressed: changeState(uiState.statePressed)
    onCanceled: changeState(uiState.stateDefault)
    onReleased: {
        if (hoverEnabled) {
            changeState(uiState.stateDefault);
            onMouseAreaClicked();
        }
    }
}
