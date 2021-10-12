/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

VPNButtonBase {
    id: mainRow

    property var rowShouldBeDisabled: false
    property var accessibleName
    property var backgroundColor: Theme.iconButtonLightBackground

    property var handleMouseClick: function() { mainRow.clicked(); }
    property var canGrowVertical: false

    visualStateItem: rowVisualStates

    height: Theme.rowHeight
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: Theme.windowMargin / 2
    anchors.rightMargin: Theme.windowMargin / 2
    width: parent.width - (Theme.windowMargin * 2)
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
        height: canGrowVertical? mainRow.height : Theme.rowHeight
        anchors.top: mainRow.top
        radius: Theme.cornerRadius
        border.width: Theme.focusBorderWidth
        border.color: "transparent"
        color: "transparent"
        Component.onCompleted: rowVisualStates.state = uiState.stateDefault
    }

    VPNUIStates {
        id: vpnFocus

        itemToAnchor: rowVisualStates
        colorScheme: backgroundColor
    }

    VPNMouseArea {
        anchors.fill: rowVisualStates
        hoverEnabled: !rowShouldBeDisabled
        targetEl: rowVisualStates
        onMouseAreaClicked: handleMouseClick
    }

}
