/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15

import Mozilla.VPN 1.0

Text {
    font.pixelSize: VPNTheme.theme.fontSizeSmallest
    font.family: VPNTheme.theme.fontInterSemiBoldFamily
    lineHeightMode: Text.FixedHeight
    lineHeight: VPNTheme.theme.controllerInterLineHeight
    color: VPNTheme.theme.fontColorDark
    wrapMode: Text.Wrap
    horizontalAlignment: Qt.AlignLeft

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
