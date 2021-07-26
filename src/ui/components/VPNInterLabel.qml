/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

// VPNInterLabel
Text {
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    font.pixelSize: Theme.fontSize
    font.family: Theme.fontInterFamily
    lineHeightMode: Text.FixedHeight
    lineHeight: Theme.labelLineHeight
    wrapMode: Text.Wrap
    color: Theme.fontColorDark

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
