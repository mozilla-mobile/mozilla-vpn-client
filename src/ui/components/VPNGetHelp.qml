/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    property alias isSettingsView: menu.isSettingsView

    VPNMenu {
        id: menu
        objectName: "getHelpBack"

        //% "Get help"
        title: qsTrId("vpn.main.getHelp2")
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
