/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import "../themes/themes.js" as Theme

// VPNTextBlock
Text {
    color: Theme.fontColor
    font.family: Theme.fontInterFamily
    font.pixelSize: Theme.fontSizeSmall
    lineHeightMode: Text.FixedHeight
    lineHeight: 21
    width: Theme.maxTextWidth
    wrapMode: Text.Wrap

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
