/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import compat 0.1

Rectangle {
    id: root
    property list<VPNBottomNavigationBarButton> tabs

    color: VPNTheme.theme.ink

    VPNDropShadow {
        id: shadow
        source: outline
        anchors.fill: outline
        transparentBorder: true
        verticalOffset: 2
        opacity: 0.6
        spread: 0
        radius: parent.radius
        color: VPNTheme.colors.grey60
        cached: true
        z: -1
    }

    Rectangle {
        id: outline
        color: VPNTheme.theme.ink
        radius: parent.radius
        anchors.fill: parent
        border.width: 0
        border.color: VPNTheme.theme.ink
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        anchors.topMargin: VPNTheme.theme.vSpacingSmall / 2
        anchors.leftMargin: (VPNTheme.theme.windowMargin * 6) - (tabs.length * 16)
        anchors.rightMargin: (VPNTheme.theme.windowMargin * 6) - (tabs.length * 16)
        anchors.bottomMargin: VPNTheme.theme.vSpacingSmall / 2

        spacing: (root.width - anchors.leftMargin - anchors.rightMargin - (48 * tabs.length)) / tabs.length
        data: tabs
    }
}
