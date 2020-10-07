/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    property alias isSettingsView: menu.isSettingsView

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
                       VPN.viewLogs();
                       break;
                   }
               }
            onClicked: clickHandler(clickId)
        }

        ScrollBar.vertical: ScrollBar {}
    }
}
