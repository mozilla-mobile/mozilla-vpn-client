/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    property alias isSettingsView: menu.isSettingsView

    VPNMenu {
        id: menu
        objectName: "getHelpBack"

        //% "Get Help"
        title: qsTrId("vpn.main.getHelp")
        isSettingsView: true
    }

    VPNList {
        objectName: "getHelpBackList"
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        spacing: Theme.listSpacing
        anchors.topMargin: Theme.windowMargin
        listName: menu.title

        model: VPNHelpModel

        delegate: VPNExternalLinkListItem {
            objectName: "getHelpBackList-" + id
            title: name
            accessibleName: name
            iconSource: externalLink ? "../resources/externalLink.svg" : "../resources/chevron.svg"
            backgroundColor: externalLink ? Theme.clickableRowBlue : Theme.iconButtonLightBackground
            onClicked: {
                VPNHelpModel.open(id)
            }
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
