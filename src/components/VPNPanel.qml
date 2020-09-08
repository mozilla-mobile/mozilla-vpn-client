import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.0

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme
Item {

    property alias logo: logo.source
    property alias logoTitle: logoTitle.text
    property alias logoSubtitle: logoSubtitle.text

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
        y: 80
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
        font.family: vpnFontInter.name
        wrapMode: Text.Wrap
        width: Theme.maxTextWidth
        color: Theme.fontColor
        lineHeightMode: Text.FixedHeight
        lineHeight: 22
    }
}
