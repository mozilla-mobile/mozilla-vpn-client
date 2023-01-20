/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    property real maxPaintedTextWidth: box.width - ipVersion.paintedWidth - VPNTheme.theme.windowMargin
    property alias ipVersionText: ipVersion.text
    property alias ipAddressText: ipAddress.text

    spacing: VPNTheme.theme.listSpacing
    Layout.fillWidth: true

    VPNBoldInterLabel {
        id: ipVersion

        color: VPNTheme.theme.white
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        lineHeight: VPNTheme.theme.labelLineHeight * 1.25
        verticalAlignment: Text.AlignVCenter

        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }

    VPNInterLabel {
        id: ipAddress

        color: VPNTheme.colors.white
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        horizontalAlignment: Text.AlignLeft
        lineHeight: VPNTheme.theme.labelLineHeight * 1.25
        opacity: 0.8

        Accessible.name: text
        Accessible.role: Accessible.StaticText
        Layout.maximumWidth: maxPaintedTextWidth
    }
}
