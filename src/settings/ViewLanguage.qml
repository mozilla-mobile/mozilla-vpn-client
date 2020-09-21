import QtQuick 2.0
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    VPNMenu {
        id: menu
        title: qsTr("Language")
        isSettingsView: true
    }

    ListView {
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        clip: true
        spacing: Theme.listSpacing
        interactive: false
        anchors.topMargin: Theme.windowMargin

        model: VPNLocalizer

        delegate: VPNExternalLinkListItem {
            title: language
            onClicked: VPNLocalizer.setLanguage(code)
        }
    }
}
