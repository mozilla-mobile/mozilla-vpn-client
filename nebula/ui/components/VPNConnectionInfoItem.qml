import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Row {
    id: infoStatusItem

    property string title: ""
    property string subtitle: ""
    property string iconPath: ""
    property bool isFlagIcon: false

    height: 40
    spacing: 0

    Layout.fillWidth: true

    Rectangle {
        id: itemIconWrapper

        anchors.verticalCenter: infoStatusItem.verticalCenter

        color: "transparent"
        height: VPNTheme.theme.iconSize * 1.5
        width: VPNTheme.theme.iconSize * 1.5

        VPNIcon {
            id: itemIcon

            anchors.centerIn: parent
            antialiasing: true
            source: infoStatusItem.iconPath
            sourceSize.width: isFlagIcon ? VPNTheme.theme.iconSizeFlag : parent.width
            sourceSize.height: isFlagIcon ? VPNTheme.theme.iconSizeFlag : parent.height
            height: isFlagIcon ? VPNTheme.theme.iconSize : parent.height
            width: isFlagIcon ? VPNTheme.theme.iconSize : parent.width
        }
    }

    VPNBoldLabel {
        id: itemLabel
        color: VPNTheme.theme.white
        leftPadding: itemIconWrapper.width + VPNTheme.theme.iconSizeSmall * 0.25
        text: infoStatusItem.title
        anchors.verticalCenter: infoStatusItem.verticalCenter
    }

    VPNInterLabel {
        id: itemValue

        color: "white"
        text: infoStatusItem.subtitle

        anchors.right: infoStatusItem.right
        anchors.verticalCenter: infoStatusItem.verticalCenter
    }

    // Rectangle {
    //     anchors.fill: parent
    //     border.color: "black"
    //     color: "orange"
    //     z: -1
    // }
}
