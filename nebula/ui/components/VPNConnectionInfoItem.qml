import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

ColumnLayout {
    id: infoStatusItem

    property string title: ""
    property string subtitle: ""
    property string iconPath: ""

    spacing: 0

    VPNIconAndLabel {
        id: itemTitle

        height: parent.height
        icon: infoStatusItem.iconPath
        title: infoStatusItem.title
        Layout.fillWidth: true
    }

    VPNTextBlock {
        id: itemSubtitle

        text: infoStatusItem.subtitle
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.vSpacing + 14
    }

    // Rectangle {
    //     anchors.fill: parent
    //     border.color: "black"
    //     color: "orange"
    //     z: -1
    // }
}
