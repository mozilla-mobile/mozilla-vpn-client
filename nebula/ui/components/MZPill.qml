/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0

Item {
    id: pill

    property string text: ""
    property string background: MZTheme.colors.bgColor
    property string color: MZTheme.colors.fontColorDark

    height: label.implicitHeight
    width: label.implicitWidth + label.font.pixelSize * 1.5

    Rectangle {
        color: pill.background
        radius: MZTheme.theme.cornerRadius

        anchors.fill: parent
    }

    MZInterLabel {
        id: label

        color: pill.color
        font.pixelSize: MZTheme.theme.fontSizeSmallest
        font.family: MZTheme.theme.fontInterSemiBoldFamily
        lineHeight: MZTheme.theme.labelLineHeight * 1.1
        text: pill.text

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: 1
        }
    }
}
