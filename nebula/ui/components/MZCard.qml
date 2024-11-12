/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Mozilla.Shared 1.0
import compat 0.1

MZButtonBase {
    id: button
    radius: 8

    MZDropShadow {
        anchors.fill: parent
        z: -1
        source: parent
        horizontalOffset: 0
        verticalOffset: 1
        radius: 8
        color: MZTheme.colors.dropShadow
        opacity: .1
        transparentBorder: true
        cached: true
    }

    MZUIStates {
        colorScheme: MZTheme.colors.card
        radius: parent.radius
    }

    MZMouseArea {
        hoverEnabled: true
        targetEl: button
    }
}
