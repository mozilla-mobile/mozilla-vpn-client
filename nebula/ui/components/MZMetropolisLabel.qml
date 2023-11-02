/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0

// MZMetropolisLabel
Text {
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    font.pixelSize: MZTheme.theme.fontSize
    font.family: MZTheme.theme.fontFamily
    lineHeightMode: Text.FixedHeight
    lineHeight: MZTheme.theme.labelLineHeight
    wrapMode: Text.Wrap

    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.ignored: !visible
}
