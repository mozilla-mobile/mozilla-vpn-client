/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNButtonBase {
    id: mainRow

    property var rowShouldBeDisabled: false
    property var accessibleName
    property var backgroundColor: VPNTheme.theme.iconButtonLightBackground

    property var handleMouseClick: function() { mainRow.clicked(); }
    property var canGrowVertical: false

    visualStateItem: rowVisualStates

    height: VPNTheme.theme.rowHeight
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: VPNTheme.theme.windowMargin / 2
    anchors.rightMargin: VPNTheme.theme.windowMargin / 2
    width: parent.width - (VPNTheme.theme.windowMargin * 2)
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
    // in VPNServerCountry {}
    Rectangle {
        id: rowVisualStates

        width: mainRow.width
        height: canGrowVertical ? mainRow.height : VPNTheme.theme.rowHeight
        anchors.top: mainRow.top
        radius: VPNTheme.theme.cornerRadius
        border.width: VPNTheme.theme.focusBorderWidth
        border.color: VPNTheme.theme.transparent
        color: VPNTheme.theme.transparent
        opacity: rowVisualStates.state === VPNTheme.theme.uiState.stateFocused
            ? 1
            : 0

        Component.onCompleted: {
            rowVisualStates.state = VPNTheme.theme.uiState.stateDefault
        }
    }

    VPNUIStates {
        id: vpnFocus

        itemToAnchor: rowVisualStates
        colorScheme: backgroundColor
    }

    VPNMouseArea {
        enabled: parent.enabled
        anchors.fill: rowVisualStates
        hoverEnabled: enabled
        targetEl: rowVisualStates
        onMouseAreaClicked: handleMouseClick
    }

}
