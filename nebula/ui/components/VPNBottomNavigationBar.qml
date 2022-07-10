import QtQuick 2.0
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

Rectangle {
    property list<Item> tabs

    color: VPNTheme.theme.ink

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        anchors.leftMargin: tabs.length === 3 ? 48 : 32
        anchors.rightMargin: anchors.leftMargin

        spacing: tabs.length === 3 ? 51.5 : 24
        data: tabs

        Component.onCompleted: {
            for(var i = 0 ; i < tabs.length; i++) {
                tabs[i].Layout.fillWidth = true
            }
        }
    }
}
