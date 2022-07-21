import QtQuick 2.0
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import compat 0.1

Rectangle {
    property list<Item> tabs

    color: VPNTheme.theme.ink

    VPNDropShadow {
        id: shadow
        source: outline
        anchors.fill: outline
        transparentBorder: true
        verticalOffset: 2
        opacity: 0.6
        spread: 0
        radius: parent.radius
        color: VPNTheme.colors.grey60
        cached: true
        z: -1
    }

    Rectangle {
        id: outline
        color: VPNTheme.theme.ink
        radius: parent.radius
        anchors.fill: parent
        border.width: 0
        border.color: VPNTheme.theme.ink
    }

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
