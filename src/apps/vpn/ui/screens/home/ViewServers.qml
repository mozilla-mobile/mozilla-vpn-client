/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    id: root
    objectName: "viewServers"


    MZMenu {
        property string defaultMenuTitle: qsTrId("vpn.servers.selectLocation")
        id: menu
        objectName: "serverListBackButton"

        title: defaultMenuTitle
        onActiveFocusChanged: if (focus) forceFocus = true

        _menuOnBackClicked: () => {
            if (multiHopStackView && multiHopStackView.depth > 1) {
                // User clicked back from either the Multi-hop entry or exit server list
                multiHopStackView.pop();
                menu.title = menu.defaultMenuTitle;
                return;
            }

            if (segmentedNav.selectedSegment.objectName === "tabMultiHop" && multiHopStackView.depth === 1) {
                // User clicked back button from the Multi-hop tab main view
                VPNCurrentServer.changeServer(...segmentedNav.multiHopExitServer.slice(0,2), ...segmentedNav.multiHopEntryServer.slice(0,2));
            }

            if (segmentedNav.selectedSegment.objectName === "tabSingleHop") {
                // User clicked back button from the Single-hop tab view but didn't select a new server
                VPNCurrentServer.changeServer(VPNCurrentServer.exitCountryCode, VPNCurrentServer.exitCityName)
            }

            return stackview.pop()
        }
    }

    Connections {
        function onGoBack(item) {
            if (getStack() === item && root.visible) {
                menu._menuOnBackClicked();
            }
        }

        target: VPNNavigator
    }

    MZSegmentedNavigation {
        id: segmentedNav

        property var multiHopEntryServer: [VPNCurrentServer.entryCountryCode, VPNCurrentServer.entryCityName, VPNCurrentServer.localizedEntryCityName]
        property var multiHopExitServer: [VPNCurrentServer.exitCountryCode, VPNCurrentServer.exitCityName, VPNCurrentServer.localizedExitCityName]

        anchors {
            top: menu.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            topMargin: MZTheme.theme.vSpacingSmall
        }

        segmentedToggleButtonsModel: ListModel {
            ListElement {
                segmentLabelStringId: "MultiHopFeatureSingleHopToggleCTA"
                segmentButtonId: "tabSingleHop"
            }
            ListElement {
                segmentLabelStringId: "MultiHopFeatureMultiHopToggleCTA"
                segmentButtonId: "tabMultiHop"
            }
        }

        stackContent: [
            MZServerList {
                id: singleHopServerList
                currentServer: {
                    "countryCode": VPNCurrentServer.exitCountryCode,
                    "localizedCityName": VPNCurrentServer.localizedExitCityName,
                    "cityName": VPNCurrentServer.exitCityName,
                    "whichHop": "singleHopServer"
                }
                showRecentConnections: true
            }
        ]

        handleSegmentClick: (segment) => {
                            if (multiHopStackView && multiHopStackView.depth > 1) {
                                // Return to the Multi-hop main view when the Multi-hop tab
                                // is clicked from a Multi-hop entry or exit server list
                                multiHopStackView.pop();
                            }

                            if (segment.objectName === "tabSingleHop") {
                                // Do single hop things
                                menu.title = menu.defaultMenuTitle;
                                singleHopServerList.centerActiveServer();
                                return;
                            }
                            else if (multiHopEntryServer[0] === "") {
                                // Choose a random entry server when switching to multihop
                                var best = VPNServerCountryModel.recommendedLocations(2);
                                if ((best[0].country != multiHopExitServer[0]) || (best[0].name != multiHopExitServer[1])) {
                                    multiHopEntryServer = [best[0].country, best[0].name, best[0].localizedName];
                                } else {
                                    multiHopEntryServer = [best[1].country, best[1].name, best[1].localizedName];
                                }
                            }
                        }


        ViewMultiHop {
            id: multiHopStackView
            visible: VPNFeatureList.get("multiHop").isSupported
        }
    }

    Component.onCompleted: {
        if (!VPNFeatureList.get("multiHop").isSupported) {
            return;
        }

        segmentedNav.stackContent.push(multiHopStackView);
        if (VPNCurrentServer.entryCountryCode && VPNCurrentServer.entryCountryCode !== "") {
            // Set default tab to multi-hop
            return segmentedNav.setSelectedIndex(1);
        }
    }
}

