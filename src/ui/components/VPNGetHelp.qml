/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    property alias isSettingsView: menu.isSettingsView

    VPNMenu {
        id: menu

        //% "Get Help"
        title: qsTrId("vpn.main.getHelp")
        isSettingsView: true
    }

    VPNList {
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        clip: true
        spacing: Theme.listSpacing
        anchors.topMargin: Theme.windowMargin
        listName: menu.title

        model: ListModel {
            ListElement {
                //% "Contact us"
                linkTitle: qsTrId("vpn.help.contactUs")
                clickId: "contact_us"
            }

            ListElement {
                //% "Help & Support"
                linkTitle: qsTrId("vpn.help.helpAndSupport")
                clickId: "help_support"
            }

            ListElement {
                //% "View log"
                linkTitle: qsTrId("vpn.help.viewLog")
                clickId: "view_logs"
            }

        }

        delegate: VPNExternalLinkListItem {

            function clickHandler(id) {
                switch (id) {
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

            title: linkTitle
            accessibleName: linkTitle
            onClicked: clickHandler(clickId)
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
