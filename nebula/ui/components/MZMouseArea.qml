/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0

MouseArea {
    id: mouseArea

    property var targetEl: parent
    property var uiState: MZTheme.theme.uiState
    property bool propagateClickToParent: true
    property var onMouseAreaClicked: () => { if(propagateClickToParent) parent.clicked() }

    // Set to true in tst_MZMouseArea.qml to prevent failing
    // at L32 `mouseArea.containsMouse`.
    property bool qmlUnitTestWorkaround: false
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
        if (hoverEnabled && (mouseArea.containsMouse || mouseArea.qmlUnitTestWorkaround)) {
            changeState(uiState.stateDefault);
            onMouseAreaClicked();
        }
    }
    onHoverEnabledChanged: {
        changeState(hoverEnabled ? uiState.stateDefault : uiState.stateDisabled);
    }
}
