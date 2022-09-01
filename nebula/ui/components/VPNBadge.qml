/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.VPN 1.0

Rectangle {
    property var theme
    property alias text: badgeLabel.text

    color: theme.backgroundColor
    height: badgeLabel.implicitHeight
    width: badgeLabel.implicitWidth
    radius: 4
    clip: true

    VPNBoldInterLabel {
        id: badgeLabel

        color: theme.textColor
        verticalAlignment: Text.AlignVCenter
        topPadding: VPNTheme.theme.badgeVerticalPadding
        leftPadding: VPNTheme.theme.badgeHorizontalPadding
        rightPadding: VPNTheme.theme.badgeHorizontalPadding
        bottomPadding: VPNTheme.theme.badgeVerticalPadding
        maximumLineCount: 1
        elide: Text.ElideRight
    }
}
