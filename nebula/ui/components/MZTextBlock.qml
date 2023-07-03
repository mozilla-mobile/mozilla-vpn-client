/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

// MZTextBlock
Text {
    color: MZTheme.theme.fontColor
    font.family: MZTheme.theme.fontInterFamily
    font.pixelSize: MZTheme.theme.fontSizeSmall
    lineHeightMode: Text.FixedHeight
    lineHeight: 21
    wrapMode: Text.Wrap
    Layout.alignment: Qt.AlignLeft
    horizontalAlignment: Qt.AlignLeft

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
