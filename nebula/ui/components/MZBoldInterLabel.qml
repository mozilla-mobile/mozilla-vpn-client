/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15

import Mozilla.Shared 1.0

Text {
    font.pixelSize: MZTheme.theme.fontSizeSmallest
    font.family: MZTheme.theme.fontInterSemiBoldFamily
    lineHeightMode: Text.FixedHeight
    lineHeight: MZTheme.theme.controllerInterLineHeight
    color: MZTheme.theme.fontColorDark
    wrapMode: Text.Wrap
    horizontalAlignment: Qt.AlignLeft

    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.ignored: !visible
}
