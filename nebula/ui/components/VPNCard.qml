/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15

import Mozilla.VPN 1.0
import compat 0.1

VPNButtonBase {
    id: button
    radius: 8

    VPNDropShadow {
        anchors.fill: parent
        z: -1
        source: parent
        horizontalOffset: 0
        verticalOffset: 1
        radius: 8
        color: VPNTheme.colors.grey60
        opacity: .1
        transparentBorder: true
        cached: true
    }

    VPNUIStates {
        colorScheme: VPNTheme.theme.card
        radius: parent.radius
    }

    VPNMouseArea {
        hoverEnabled: true
        targetEl: button
    }
}
