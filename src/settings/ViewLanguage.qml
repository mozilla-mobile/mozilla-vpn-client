import QtQuick 2.0
import Mozilla.VPN 1.0

import "../components"
Item {
    VPNMenu {
        id: menu
        title: qsTr("Language")
        isSettingsView: true
    }
}
