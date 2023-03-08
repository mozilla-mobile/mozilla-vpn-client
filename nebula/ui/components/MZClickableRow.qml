/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import components 0.1

MZButtonBase {
    id: mainRow

    property bool rowShouldBeDisabled: false
    property string accessibleName: ""
    property var backgroundColor: MZTheme.theme.iconButtonLightBackground

    property var handleMouseClick: function() { mainRow.clicked(); }
    property bool canGrowVertical: false

    visualStateItem: rowVisualStates

    height: MZTheme.theme.rowHeight
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: MZTheme.theme.windowMargin / 2
    anchors.rightMargin: MZTheme.theme.windowMargin / 2
    width: parent.width - (MZTheme.theme.windowMargin * 2)
    opacity: rowShouldBeDisabled ? 0.7 : 1
    enabled: !rowShouldBeDisabled

    Accessible.ignored: rowShouldBeDisabled
    Accessible.name: accessibleName

    Behavior on opacity {
        PropertyAnimation {
            duration: 100
        }
    }

    // visual state changes are applied to this
    // component to prevent state overwrite conflicts
    // in ServerCountry {}
    Rectangle {
        id: rowVisualStates

        width: mainRow.width
        height: canGrowVertical ? mainRow.height : MZTheme.theme.rowHeight
        anchors.top: mainRow.top
        radius: MZTheme.theme.cornerRadius
        border.width: MZTheme.theme.focusBorderWidth
        border.color: MZTheme.theme.transparent
        color: MZTheme.theme.transparent
        opacity: rowVisualStates.state === MZTheme.theme.uiState.stateFocused
            ? 1
            : 0

        Component.onCompleted: {
            rowVisualStates.state = MZTheme.theme.uiState.stateDefault
        }
    }

    MZUIStates {
        id: vpnFocus

        itemToAnchor: rowVisualStates
        colorScheme: backgroundColor
    }

    MZMouseArea {
        enabled: parent.enabled
        anchors.fill: rowVisualStates
        hoverEnabled: enabled
        targetEl: rowVisualStates
        onMouseAreaClicked: handleMouseClick
    }

}
