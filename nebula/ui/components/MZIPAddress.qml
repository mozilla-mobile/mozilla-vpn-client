/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

RowLayout {
    property real maxPaintedTextWidth: box.width - ipVersion.paintedWidth - MZTheme.theme.windowMargin
    property alias ipVersionText: ipVersion.text
    property alias ipAddressText: ipAddress.text

    spacing: MZTheme.theme.listSpacing
    Layout.fillWidth: true

    MZBoldInterLabel {
        id: ipVersion

        color: MZTheme.theme.white
        font.pixelSize: MZTheme.theme.fontSizeSmall
        lineHeight: MZTheme.theme.labelLineHeight * 1.25
        verticalAlignment: Text.AlignVCenter

        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }

    MZInterLabel {
        id: ipAddress

        color: MZTheme.colors.white
        font.pixelSize: MZTheme.theme.fontSizeSmall
        horizontalAlignment: Text.AlignLeft
        lineHeight: MZTheme.theme.labelLineHeight * 1.25
        opacity: 0.8

        Accessible.name: text
        Accessible.role: Accessible.StaticText
        Layout.maximumWidth: maxPaintedTextWidth
    }
}
