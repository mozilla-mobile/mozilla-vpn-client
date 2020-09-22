import QtQuick 2.0
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Language")
        isSettingsView: true
    }

    VPNExternalLinkListItem {
        id: systemLanguage
        anchors.top: menu.bottom
        width: parent.width
        title: "SYSTEM:" + VPNLocalizer.systemLanguage + " (" + VPNLocalizer.systemLocalizedLanguage + ")";
        onClicked: VPNLocalizer.setLanguage("");
    }

    ListView {
        height: parent.height - menu.height - systemLanguage.height
        width: parent.width
        anchors.top: systemLanguage.bottom
        clip: true
        spacing: Theme.listSpacing
        interactive: false
        anchors.topMargin: Theme.windowMargin

        model: VPNLocalizer

        delegate: VPNExternalLinkListItem {
            title: language + " (" + localizedLanguage + ")"
            onClicked: VPNLocalizer.setLanguage(code)
        }
    }

    ScrollBar.vertical: ScrollBar {}
}
