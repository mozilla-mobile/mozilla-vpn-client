/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.2
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: root
    VPNMenu {
        id: menu

        title: qsTrId("vpn.servers.selectLocation")
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    DelegateModel {
        id: delegateModel

        model: VPNServerCountryModel
        delegate: VPNServerCountry {}
    }

    VPNList {
        id: serverList

        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        spacing: Theme.listSpacing
        clip: true
        listName: menu.title
        interactive: true

        onCurrentIndexChanged: currentItem.forceActiveFocus()
        header: Rectangle {
            height: 16
            width: serverList.width
            color: "transparent"
        }
        model: delegateModel

        Component.onCompleted: {
            if (VPNController.state === VPNController.StateSwitching) {
                return;
            }

            serverList.currentIndex = (() => {
               for (let idx = 0; idx < serverList.count; idx++) {
                   if (delegateModel.items.get(idx).model.code === VPNCurrentServer.countryCode) {
                       return idx;
                   }
               }
            })();
            serverList.positionViewAtIndex(serverList.currentIndex, ListView.Center);

        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
