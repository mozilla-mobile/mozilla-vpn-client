/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import themes 0.1

Item {
    id: root

    property bool isMainStackView: false
    property var onMenuBack: ({})

    VPNMenu {
        property string defaultMenuTitle: qsTrId("vpn.servers.selectLocation")
        id: menu
        objectName: "serverListBackButton"

        title: defaultMenuTitle
        onActiveFocusChanged: if (focus) forceFocus = true
        isMultiHopView: true

        function handleMultiHopNav() {
            if (multiHopStackView && multiHopStackView.depth > 1) {
                // User clicked back from either the Multi-hop entry or exit server list
                multiHopStackView.pop();
                menu.title = menu.defaultMenuTitle;
                return;
            }

            if (serversTabs.currentTab.objectName === "tabMultiHop" && multiHopStackView.depth === 1) {
                // User clicked back button from the Multi-hop tab main view
                VPNController.changeServer(...serversTabs.multiHopExitServer.slice(0,2), ...serversTabs.multiHopEntryServer.slice(0,2));
            }

            if (serversTabs.currentTab.objectName === "tabSingleHop") {
                // User clicked back button from the Single-hop tab view but didn't select a new server
                VPNController.changeServer(VPNCurrentServer.exitCountryCode, VPNCurrentServer.exitCityName);
            }

            if (isMainStackView && onMenuBack) {
                isMainStackView = false;
                onMenuBack();
                return;
            }

            return stackview.pop();
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
        property var multiHopEntryServer: [VPNCurrentServer.entryCountryCode, VPNCurrentServer.entryCityName, VPNCurrentServer.localizedEntryCity]
        property var multiHopExitServer: [VPNCurrentServer.exitCountryCode, VPNCurrentServer.exitCityName, VPNCurrentServer.localizedCityName]

        id: serversTabs

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
                    "whichHop": "singleHopServer"
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
                // Return to the Multi-hop main view when the Multi-hop tab
                // is clicked from a Multi-hop entry or exit server list
                multiHopStackView.pop();
            }

            if (tab.objectName === "tabSingleHop") {
                // Do single hop things
                menu.title = menu.defaultMenuTitle;
                singleHopServerList.centerActiveServer();
                return;
            }
            else if (multiHopEntryServer[0] === "") {
                // Choose a random entry server when switching to multihop
                multiHopEntryServer = VPNServerCountryModel.pickRandom();
            }
        }
    }

    Component.onCompleted: {
        if (!VPNFeatureList.get("multiHop").isSupported) {
            return;
        }

        serversTabs.stackContent.push(multiHopStackView);
        if (VPNCurrentServer.entryCountryCode && VPNCurrentServer.entryCountryCode !== "") {
            // Set default tab to multi-hop
            return serversTabs.setCurrentTabIndex(1);
        }
    }
}

