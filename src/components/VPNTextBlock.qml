import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

// VPNTextBlock
Text {
    color: Theme.fontColor
    font.family: vpnFontInter.name
    font.pixelSize: Theme.fontSizeSmall
    lineHeightMode: Text.FixedHeight
    lineHeight: 21
    width: Theme.maxTextWidth
    wrapMode: Text.Wrap
}
