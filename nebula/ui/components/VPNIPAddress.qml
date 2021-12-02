/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

RowLayout {
    property real maxPaintedTextWidth: box.width - ipVersion.paintedWidth - Theme.windowMargin
    property alias ipVersionText: ipVersion.text
    property alias ipAddressText: ipAddress.text

    spacing: 4
    anchors.horizontalCenter: parent.horizontalCenter

    VPNBoldLabel {
        id: ipVersion
        color: Theme.white
        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }
    VPNLightLabel {
        id: ipAddress

        color: Theme.bgColor
        opacity: .8
        minimumPixelSize: Theme.fontSizeSmall / 2
        Accessible.name: text
        Accessible.role: Accessible.StaticText
        verticalAlignment: Text.AlignVCenter
        Layout.maximumWidth: maxPaintedTextWidth
        textFormat: Text.PlainText
        fontSizeMode: Text.HorizontalFit
    }
}
