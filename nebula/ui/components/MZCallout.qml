/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

RowLayout {
    property alias calloutCopy: callout.text
    property alias calloutImage: img.source
    property alias color: callout.color
    property alias fontPixelSize: callout.font.pixelSize
    property alias iconSize: img.sourceSize.width

    spacing: MZTheme.theme.windowMargin
    Layout.alignment: Qt.AlignHCenter
    Layout.leftMargin: Math.min(window.width * .10, MZTheme.theme.rowHeight)
    Layout.rightMargin: Layout.leftMargin

    Rectangle {
        Layout.preferredHeight: MZTheme.theme.iconSize * 1.5
        Layout.preferredWidth: MZTheme.theme.iconSize * 1.5
        color: MZTheme.theme.transparent

        MZIcon {
            id: img
            sourceSize.width: MZTheme.theme.iconSize
            sourceSize.height: sourceSize.width
            antialiasing: true
            anchors.centerIn: parent
        }
    }

    MZTextBlock {
        id: callout
        color: MZTheme.theme.fontColorDark
        Layout.fillWidth: true
        font.pixelSize: MZTheme.theme.fontSize
        verticalAlignment: Text.AlignVCenter
        width: undefined
    }
}
