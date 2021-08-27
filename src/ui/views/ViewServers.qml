/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

Item {
    id: root


    VPNMenu {
        property string defaultMenuTitle: qsTrId("vpn.servers.selectLocation")
        id: menu
        objectName: "serverListBackButton"

        title: defaultMenuTitle
        onActiveFocusChanged: if (focus) forceFocus = true
        isMultiHopView: true

        function handleMultiHopNav() {
            if (multiHopStackView && multiHopStackView.depth > 1) {
                multiHopStackView.pop();
                menu.title = menu.defaultMenuTitle;
                return;
            }
            return stackview.pop()
        }
    }

    ListModel {
        id: tabButtonList
        ListElement {
            tabLabelStringId: "MultiHopFeatureSingleHopToggleCTA"
            tabButtonId: "tabSingleHop"
        }
        ListElement {
            tabLabelStringId: "MultiHopFeatureMultiHopToggleCTA"
            tabButtonId: "tabMultiHop"
        }
    }

    VPNTabNavigation {

        id: tabNavigation
        tabList: tabButtonList
        width: root.width
        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.height - menu.height

        stackContent: [
            VPNServerList {
                id: singleHopServerList
                currentServer: {
                    "countryCode": VPNCurrentServer.exitCountryCode,
                    "localizedCityName": VPNCurrentServer.localizedCityName,
                    "cityName": VPNCurrentServer.exitCityName,
                    "selectWhichHop": "singleHop"
                }
                showRecentConnections: true

            }
        ]

        ViewMultiHop {
            id: multiHopStackView
            visible: VPNFeatureList.get("multiHop").isSupported
        }
        handleTabClick: (tab) => {
            if (multiHopStackView && multiHopStackView.depth > 1) {
                    multiHopStackView.pop();
                }

            if (tab.objectName === "tabSingleHop") {
                // Do single hop things
                VPNController.changeServer(VPNCurrentServer.exitCountryCode, VPNCurrentServer.exitCityName)
                menu.title = menu.defaultMenuTitle;
                singleHopServerList.centerActiveServer();
                return;
            }
            // Do Multi-hop things
            VPNController.changeServer(VPNCurrentServer.exitCountryCode, VPNCurrentServer.exitCityName, "us", "Dallas, TX"); // Need a "pick random" or "pick last active multi-hop connection"

        }
    }


    Component.onCompleted: {
        if (!VPNFeatureList.get("multiHop").isSupported) {
            return;
        }

        tabNavigation.stackContent.push(multiHopStackView);
      if (VPNCurrentServer.entryCountryCode && VPNCurrentServer.entryCountryCode !== "") {
          // Set default tab to multi-hop
          tabNavigation.setCurrentTabIndex(1)
      }
    }
}

