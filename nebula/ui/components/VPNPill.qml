/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

Item {
    id: pill

    property string text: ""
    property string background: VPNTheme.theme.bgColor
    property string color: VPNTheme.theme.fontColorDark

    height: label.implicitHeight
    width: label.implicitWidth + label.font.pixelSize * 1.5

    Rectangle {
        color: pill.background
        radius: VPNTheme.theme.cornerRadius

        anchors.fill: parent
    }

    VPNInterLabel {
        id: label

        color: pill.color
        font.pixelSize: VPNTheme.theme.fontSizeSmallest
        font.family: VPNTheme.theme.fontInterSemiBoldFamily
        lineHeight: VPNTheme.theme.labelLineHeight * 1.1
        text: pill.text

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: 1
        }
    }
}
