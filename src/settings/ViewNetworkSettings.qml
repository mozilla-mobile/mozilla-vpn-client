import QtQuick 2.0
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

import "../components"

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Network settings")
        isSettingsView: true
    }

    CheckBox {
        checked: VPNSettings.ipv6
        text: qsTr("IPv6 enabled")
        onClicked: VPNSettings.ipv6 = checked
        y: menu.y + menu.height
    }

    ScrollBar.vertical: ScrollBar {}
}
