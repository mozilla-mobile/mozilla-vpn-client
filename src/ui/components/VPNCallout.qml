/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RowLayout {
    id: callout

    property var calloutTitle
    property var calloutSubtitle
    property var calloutImage

    Layout.minimumHeight: 40
    spacing: 0
    Layout.alignment: Qt.AlignHCenter

    Rectangle {
        Layout.minimumHeight: Theme.vSpacing
        Layout.minimumWidth: Theme.vSpacing
        Layout.maximumWidth: Theme.vSpacing
        color: "transparent"
        Layout.alignment: Qt.AlignTop
        Layout.topMargin: 2
        Layout.leftMargin: Theme.windowMargin * 2

        VPNIcon {
            source: calloutImage
            sourceSize.width: Theme.iconSize
            sourceSize.height: Theme.iconSize
            antialiasing: true
            Layout.alignment: Qt.AlignCenter
        }

    }

    Column {
        id: calloutCopy

        spacing: 2
        Layout.leftMargin: Theme.windowMargin
        Layout.rightMargin: Theme.windowMargin * 2
        Layout.alignment: Qt.AlignLeft
        Layout.fillWidth: true

        VPNTextBlock {
            color: Theme.fontColorDark
            Layout.fillWidth: true
            width: calloutCopy.width
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            font.pixelSize: Theme.fontSize
            text: calloutTitle
        }

        VPNTextBlock {
            color: Theme.fontColor
            font.pixelSize: Theme.fontSizeSmall
            Layout.fillWidth: true
            width: calloutCopy.width
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            text: calloutSubtitle
        }

    }

}
