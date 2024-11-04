/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0

// MZBoldLabel
Label {
    font.pixelSize: MZTheme.theme.fontSize
    font.family: MZTheme.theme.fontBoldFamily
    color: MZTheme.colors.fontColorDark
    horizontalAlignment: Qt.AlignLeft

    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.ignored: !visible
}
