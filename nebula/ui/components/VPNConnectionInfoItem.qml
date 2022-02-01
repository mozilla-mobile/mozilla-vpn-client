import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Row {
    id: infoStatusItem

    property string title: ""
    property string subtitle: ""
    property string iconPath: ""

    height: 40
    spacing: 0

    Layout.fillWidth: true

    VPNIconAndLabel {
        id: itemTitle

        fontColor: "white"
        icon: infoStatusItem.iconPath
        title: infoStatusItem.title

        anchors.verticalCenter: infoStatusItem.verticalCenter
    }

    VPNInterLabel {
        id: itemValue

        color: "white"
        text: infoStatusItem.subtitle

        anchors.right: infoStatusItem.right
        anchors.verticalCenter: infoStatusItem.verticalCenter
    }

    Rectangle {
        anchors.fill: parent
        border.color: "black"
        color: "orange"
        z: -1
    }
}
