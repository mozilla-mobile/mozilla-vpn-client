/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

// VPNSubtitle
Text {
    horizontalAlignment: Text.AlignHCenter
    font.pixelSize: Theme.fontSize
    font.family: Theme.fontInterFamily
    wrapMode: Text.Wrap
    width: Theme.maxTextWidth
    color: Theme.fontColor
    lineHeightMode: Text.FixedHeight
    lineHeight: Theme.labelLineHeight
}
