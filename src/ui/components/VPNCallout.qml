/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

RowLayout {
    property alias calloutCopy: callout.text
    property alias calloutImage: img.source

    spacing: Theme.windowMargin
    Layout.alignment: Qt.AlignHCenter
    Layout.leftMargin: Math.min(window.width * .10, Theme.rowHeight)
    Layout.rightMargin: Layout.leftMargin

    Rectangle {
        Layout.preferredHeight: 24
        Layout.preferredWidth: 24
        color: "transparent"

        VPNIcon {
            id: img
            sourceSize.width: Theme.iconSize
            sourceSize.height: Theme.iconSize
            antialiasing: true
            anchors.centerIn: parent
        }
    }

    VPNTextBlock {
        id: callout
        color: Theme.fontColorDark
        Layout.fillWidth: true
        font.pixelSize: Theme.fontSize
        verticalAlignment: Text.AlignVCenter
        width: undefined
    }
}
