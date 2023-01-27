/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    property alias calloutCopy: callout.text
    property alias calloutImage: img.source
    property alias color: callout.color
    property alias fontPixelSize: callout.font.pixelSize
    property alias iconSize: img.sourceSize.width

    spacing: VPNTheme.theme.windowMargin
    Layout.alignment: Qt.AlignHCenter
    Layout.leftMargin: Math.min(window.width * .10, VPNTheme.theme.rowHeight)
    Layout.rightMargin: Layout.leftMargin

    Rectangle {
        Layout.preferredHeight: VPNTheme.theme.iconSize * 1.5
        Layout.preferredWidth: VPNTheme.theme.iconSize * 1.5
        color: VPNTheme.theme.transparent

        VPNIcon {
            id: img
            sourceSize.width: VPNTheme.theme.iconSize
            sourceSize.height: sourceSize.width
            antialiasing: true
            anchors.centerIn: parent
        }
    }

    VPNTextBlock {
        id: callout
        color: VPNTheme.theme.fontColorDark
        Layout.fillWidth: true
        font.pixelSize: VPNTheme.theme.fontSize
        verticalAlignment: Text.AlignVCenter
        width: undefined
    }
}
