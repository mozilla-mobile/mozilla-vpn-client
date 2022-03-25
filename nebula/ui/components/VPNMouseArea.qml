/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

MouseArea {
    id: mouseArea

    property var targetEl: parent
    property var uiState: VPNTheme.theme.uiState
    property bool propagateClickToParent: true
    property var onMouseAreaClicked: function() { if(propagateClickToParent) parent.clicked() }

    function changeState(stateName) {
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
    onHoverEnabledChanged: {
        changeState(hoverEnabled ? uiState.stateDefault : uiState.stateDisabled);
    }
}
