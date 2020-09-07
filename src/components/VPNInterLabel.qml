import QtQuick 2.0
import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

// VPNInterLabel
Text {
    horizontalAlignment: Text.AlignHCenter
    font.pixelSize: Theme.fontSize
    font.family: vpnFontInter.name
    lineHeightMode: Text.FixedHeight
    lineHeight: Theme.labelLineHeight
}
