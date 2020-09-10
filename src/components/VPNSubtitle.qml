import QtQuick 2.0
import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

// VPNSubtitle
Text {
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.top: logoTitle.bottom
    anchors.topMargin: 8
    horizontalAlignment: Text.AlignHCenter
    font.pixelSize: Theme.fontSize
    font.family: vpnFontInter.name
    wrapMode: Text.Wrap
    width: Theme.maxTextWidth
    color: Theme.fontColor
    lineHeightMode: Text.FixedHeight
    lineHeight: Theme.labelLineHeight
}
