import QtQuick 2.0
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Row {
    property alias ipVersionText: ipVersion.text
    property alias ipAddressText: ipAddress.text

    spacing: 4
    anchors.horizontalCenter: parent.horizontalCenter

    VPNBoldLabel {
        id: ipVersion
        color: Theme.white
        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }
    VPNLightLabel {
        id: ipAddress

        color: Theme.bgColor
        opacity: .8
        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }
}
