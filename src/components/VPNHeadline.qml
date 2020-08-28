import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

//VPNHeadline
Text {
    horizontalAlignment: Text.AlignHCenter
    anchors.horizontalCenterOffset: 1
    anchors.horizontalCenter: parent.horizontalCenter
    wrapMode: Text.Wrap
    width: Theme.maxTextWidth

    color: Theme.fontColorDark
    font.family: vpnFont.name
    font.pixelSize: 22
    lineHeightMode: Text.FixedHeight
    lineHeight: 32

}
