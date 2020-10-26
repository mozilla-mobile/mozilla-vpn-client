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
    VPNMenu {
        id: menu

        title: qsTrId("vpn.servers.selectLocation")
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    VPNList {
        id: serverList

        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        spacing: Theme.listSpacing
        clip: true
        model: VPNServerCountryModel
        listName: menu.title
        interactive: true
        onActiveFocusChanged: {
            if (activeFocus) {
                currentItem.forceActiveFocus();
            }
        }
        onCurrentIndexChanged: currentItem.forceActiveFocus()

        highlight: VPNFocus {
            itemToFocus: serverList
            itemToAnchor: serverList.currentItem
            z: 2
            opacity: serverList.currentItem.isActive ? 0 : 1
        }

        header: Rectangle {
            height: 16
            width: serverList.width
            color: "transparent"
        }

        delegate: VPNServerCountry {}

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
