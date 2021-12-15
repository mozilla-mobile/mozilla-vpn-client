/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

//VPNHeadline
Text {
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.WordWrap
    width: VPNTheme.theme.maxTextWidth
    color: VPNTheme.theme.fontColorDark
    font.family: VPNTheme.theme.fontFamily
    font.pixelSize: 22
    lineHeightMode: Text.FixedHeight
    lineHeight: 32
    Layout.alignment: Qt.AlignHCenter
    Accessible.role: Accessible.StaticText
    Accessible.name: text

    Component.onCompleted: {
        if (paintedWidth > VPNTheme.theme.maxTextWidth) {
            fontSizeMode = Text.Fit
            minimumPixelSize = 16
        }
    }
}
