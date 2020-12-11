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

        footer: Rectangle {
            height: fullscreenRequired() ? Theme.rowHeight * 3: Theme.rowHeight * 2
            color: "transparent"
            width: serverList.width
        }

        Component.onCompleted: {
            for (let idx = 0; idx < serverList.count; idx++) {
                if (delegateModel.items.get(idx).model.code === VPNCurrentServer.countryCode) {
                    serverList.currentIndex = idx;
                    serverList.positionViewAtIndex(idx, ListView.Center);

                    const citiesList = delegateModel.items.get(idx).model.cities;
                    const selectedCityIndex = citiesList.indexOf(VPNCurrentServer.city);
                    if (citiesList.length >= 2) {
                        const delegateSpacing = Theme.rowHeight * 2
                        serverList.contentY += delegateSpacing + (selectedCityIndex * delegateSpacing);
                    }
                    return;
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
