/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

// VPNSubtitle
Text {
    horizontalAlignment: Text.AlignHCenter
    font.pixelSize: VPNTheme.theme.fontSize
    font.family: VPNTheme.theme.fontInterFamily
    wrapMode: Text.Wrap
    width: VPNTheme.theme.maxTextWidth
    color: VPNTheme.theme.fontColor
    lineHeightMode: Text.FixedHeight
    lineHeight: VPNTheme.theme.labelLineHeight
    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
