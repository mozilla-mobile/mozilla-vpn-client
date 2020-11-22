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
        onActiveFocusChanged: {
            if (activeFocus) {
                currentItem.forceActiveFocus();
            }
        }
        onCurrentIndexChanged: currentItem.forceActiveFocus()

        header: Rectangle {
            height: 16
            width: serverList.width
            color: "transparent"
        }
        model: delegateModel

        Component.onCompleted: {
            const getCurrentServerIndex = () => {
                for (var x = 0; x < serverList.count; x++) {
                    if (delegateModel.items.get(x).model.code === VPNCurrentServer.countryCode) {
                        return x;
                    }
                }
            };
            serverList.currentIndex = getCurrentServerIndex();
            serverList.currentItem.cityListVisible = true;
            serverList.currentItem.forceActiveFocus();
            serverList.positionViewAtIndex(serverList.currentIndex, ListView.Center);

        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
