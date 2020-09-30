/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

//VPNHeadline
Text {
    horizontalAlignment: Text.AlignHCenter
    anchors.horizontalCenterOffset: 1
    anchors.horizontalCenter: parent.horizontalCenter
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.Wrap
    width: Theme.maxTextWidth

    color: Theme.fontColorDark
    font.family: vpnFont.name
    font.pixelSize: 22
    lineHeightMode: Text.FixedHeight
    lineHeight: 32
}
