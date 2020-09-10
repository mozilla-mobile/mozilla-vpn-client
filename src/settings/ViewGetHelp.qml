import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    VPNMenu {
        id: menu
        title: qsTr("Get help")
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

        model: ListModel {
            ListElement {
                linkTitle: qsTr("Contact us")
                clickId: "contact_us"
            }

            ListElement {
                linkTitle: qsTr("Help & Support")
                clickId: "help_support"
            }
            
            ListElement {
                linkTitle: qsTr("Debug")
                clickId: "debug"
            }
            ListElement {
                linkTitle: qsTr("View log")
                clickId: "view_logs"
            }
        }

        delegate: VPNExternalLinkListItem {
            title: linkTitle

            function clickHandler(id) {
                   switch(id) {
                   case "contact_us":
                       VPN.openLink(VPN.LinkContact);
                       break;
                   case "help_support":
                       VPN.openLink(VPN.LinkHelpSupport);
                       break;
                   case "view_logs":
                       VPNLogger.viewLogs();
                       break;
                   }
               }
            onClicked: clickHandler(clickId)
        }
    }
}
