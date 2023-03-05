/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

//MZHeadline
Text {
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.WordWrap
    width: MZTheme.theme.maxTextWidth
    color: MZTheme.theme.fontColorDark
    font.family: MZTheme.theme.fontFamily
    font.pixelSize: 22
    lineHeightMode: Text.FixedHeight
    lineHeight: 32
    Layout.alignment: Qt.AlignHCenter
    Accessible.role: Accessible.StaticText
    Accessible.name: text

    Component.onCompleted: {
        if (paintedWidth > MZTheme.theme.maxTextWidth) {
            fontSizeMode = Text.Fit
            minimumPixelSize = 16
        }
    }
}
