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

    spacing: 4

    VPNBoldLabel {
        id: ipVersion
        color: VPNTheme.theme.white
        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }
    VPNLightLabel {
        id: ipAddress

        Layout.topMargin: 1
        color: VPNTheme.theme.bgColor
        opacity: .8
        minimumPixelSize: VPNTheme.theme.fontSizeSmall / 2
        Accessible.name: text
        Accessible.role: Accessible.StaticText
        Layout.maximumWidth: maxPaintedTextWidth
        textFormat: Text.PlainText
        fontSizeMode: Text.HorizontalFit
    }
}
