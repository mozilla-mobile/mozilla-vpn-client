/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme
Item {

    property alias logo: logo.source
    property alias logoTitle: logoTitle.text
    property alias logoSubtitle: logoSubtitle.text

    property var logoY: 80

    anchors.horizontalCenter: parent.horizontalCenter

    Rectangle {
        // We nest the panel Image inside this Rectangle to prevent
        // logoTitle and logoSubittle from wiggling when Image.height
        // changes as Image.src is updated.

        id: logoWrapper
        color: "transparent"
        height: 76
        width: 76
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: logoY
        Image {
            id: logo
            anchors.horizontalCenter: parent.horizontalCenter
            sourceSize.height: 76
            fillMode: Image.PreserveAspectFit
        }
    }

    VPNHeadline {
        id: logoTitle
        anchors.top: logoWrapper.bottom
        anchors.topMargin: 24
    }

    Text {
        id: logoSubtitle
        x: 169
        y: 255
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoTitle.bottom
        anchors.topMargin: 8
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 15
        font.family: Theme.fontInterFamily
        wrapMode: Text.Wrap
        width: Theme.maxTextWidth
        color: Theme.fontColor
        lineHeightMode: Text.FixedHeight
        lineHeight: 22
    }
}
