import QtQuick 2.0
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

import "../components"

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Notifications")
        isSettingsView: true
    }

    ScrollBar.vertical: ScrollBar {}
}
