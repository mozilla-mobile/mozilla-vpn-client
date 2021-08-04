/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: root
    VPNMenu {
        id: menu
        objectName: "serverListBackButton"

        title: qsTrId("vpn.servers.selectLocation")
        onActiveFocusChanged: if (focus) forceFocus = true
    }

    FocusScope {
        id: focusScope

        property var lastFocusedItemIdx

        height: parent.height - menu.height
        anchors.top: menu.bottom
        width: parent.width
        onActiveFocusChanged: if (focus && lastFocusedItemIdx) repeater.itemAt(lastFocusedItemIdx).forceActiveFocus()
        Accessible.name: menu.title
        Accessible.role: Accessible.List

        ButtonGroup {
            id: radioButtonGroup
        }


        VPNFlickable {
            id: vpnFlickable
            objectName: "serverCountryView"

            flickContentHeight: serverList.y + serverList.implicitHeight + (Theme.rowHeight * 2)
            anchors.fill: parent

            Rectangle {
                id: verticalSpacer

                height: Theme.windowMargin / 2
                width: parent.width
                color: "transparent"
            }

            NumberAnimation on contentY {
                        id: scrollAnimation

                        duration: 200
                        easing.type: Easing.OutQuad
                    }

            Column {
                id: serverList
                objectName: "serverCountryList"

                spacing: 14
                width: parent.width
                anchors.top: verticalSpacer.bottom
                Component.onCompleted: {

                    // Scroll vpnFlickable so that the current server city is
                    // vertically centered in the view

                    const serverListYCenter = vpnFlickable.height / 2;

                    for (let idx = 0; idx < repeater.count; idx++) {
                        const countryItem = repeater.itemAt(idx);
                        const countryItemYPosition = countryItem.mapToItem(vpnFlickable.contentItem, 0, 0).y;
                        if (!countryItem.cityListVisible || countryItemYPosition < serverListYCenter) {
                            continue;
                        }

                        const currentCityYPosition = countryItem.y + (Theme.rowHeight * 2) + (54 * countryItem.currentCityIndex) - serverListYCenter;
                        const destinationY = (currentCityYPosition + vpnFlickable.height > vpnFlickable.contentHeight) ? vpnFlickable.contentHeight - vpnFlickable.height : currentCityYPosition;

                        vpnFlickable.contentY = destinationY;
                        return;
                    }
                }

                Repeater {
                    id: repeater
                    model: VPNServerCountryModel
                    delegate: VPNServerCountry{}
                }
            }
        }
    }
}
