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

FocusScope {
    id: focusScope

    property real listOffset: (VPNTheme.theme.menuHeight * 2)
    property bool showRecentConnections: false
    property var currentServer

    ListModel {
        id: testRecommendedModel

        ListElement {
            countryCode: "ca"
            cityName: "Toronto"
            localizedCityName: "Toronto"
        }
        ListElement {
            countryCode: "ca"
            cityName: "Montreal"
            localizedCityName: "Montreal"
        }
        ListElement {
            countryCode: "ca"
            cityName: "Vancouver"
            localizedCityName: "Vancouver"
        }
        ListElement {
            countryCode: "us"
            cityName: "New York City"
            localizedCityName: "New York City"
        }
        ListElement {
            countryCode: "us"
            cityName: "Chicago"
            localizedCityName: "Chicago"
        }
    }

    function setSelectedServer(countryCode, cityName, localizedCityName) {
        if (currentServer.whichHop === "singleHopServer") {
            VPNController.changeServer(countryCode, cityName);
            stackview.pop();
            return;
        }

        segmentedNav[currentServer.whichHop] = [countryCode, cityName, localizedCityName];
        multiHopStackView.pop();
    }

    function centerActiveServer(serverListFlickable) {
        // Scroll vpnFlickable so that the current server city is
        // vertically centered in the view
        const serverListYCenter = serverListFlickable.height / 2 - listOffset

        for (let idx = 0; idx < serverListFlickable.countries.count; idx++) {
            const countryItem = serverListFlickable.countries.itemAt(idx);

            if (
                // Country does not host current active server
                countryItem._countryCode !== currentServer.countryCode ||
                // Country is already above the vertical center
                countryItem.y < serverListYCenter
            ) {
                continue;
            }

            // Get distance to the current server city and scroll
            const currentCityYPosition = (countryItem.y
                + VPNTheme.theme.cityListTopMargin * 3 * countryItem.currentCityIndex
                - serverListYCenter);
            const destinationY = (currentCityYPosition + serverListFlickable.height > serverListFlickable.contentHeight)
                ? serverListFlickable.contentHeight - serverListFlickable.height
                : currentCityYPosition;
            serverListFlickable.contentY = destinationY;

            if (!countryItem.cityListVisible) {
                countryItem.openCityList();
            }

            return;
        }
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Accessible.name: menu.title
    Accessible.role: Accessible.List

    ButtonGroup {
        id: radioButtonGroup
    }

    // Recommended servers component
    Component {
        id: listServersRecommended

        VPNFlickable {
            objectName: "serverCountryViewRecommend"
            id: vpnFlickableRecommended

            flickContentHeight: serverListRecommended.implicitHeight + listOffset
            anchors.fill: parent

            Column {
                id: serverListRecommended
                spacing: VPNTheme.theme.listSpacing
                width: parent.width

                anchors {
                    top: parent.top
                    topMargin: VPNTheme.theme.vSpacingSmall
                    left: parent.left
                    right: parent.right
                }

                VPNCollapsibleCard {
                    title: VPNl18n.ServersViewRecommendedCardTitle

                    anchors.horizontalCenter: parent.horizontalCenter

                    iconSrc: "qrc:/ui/resources/tip.svg"
                    contentItem: VPNTextBlock {
                        text: VPNl18n.ServersViewRecommendedCardBody
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                    }
                    width: parent.width - VPNTheme.theme.windowMargin * 2
                }

                Repeater {
                    id: recommendedRepeater
                    model: testRecommendedModel
                    delegate: VPNClickableRow {
                        onClicked: {
                            focusScope.setSelectedServer(countryCode, cityName, localizedCityName);
                        }

                        RowLayout {
                            anchors {
                                centerIn: parent
                                left: parent.left
                                right: parent.right
                            }
                            width: parent.width - VPNTheme.theme.windowMargin * 2
                            height: parent.height

                            VPNServerLabel {
                                id: recommendedServer
                                serversList: [{
                                    countryCode,
                                    cityName,
                                    localizedCityName
                                }]
                            }
                        }
                    }
                }
            }
        }
    }

    // All servers component
    Component {
        id: listServersAll

        VPNFlickable {
            objectName: "serverCountryView"
            property alias countries: countriesRepeater
            id: vpnFlickable

            flickContentHeight: serverList.implicitHeight + listOffset
            anchors.fill: parent

            Rectangle {
                id: verticalSpacer

                height: VPNTheme.theme.vSpacing
                width: parent.width
                color: VPNTheme.theme.transparent
            }

            NumberAnimation on contentY {
                id: scrollAnimation

                duration: 200
                easing.type: Easing.OutQuad
            }

            Column {
                id: serverList
                objectName: "serverCountryList"

                spacing: VPNTheme.theme.listSpacing
                width: parent.width
                anchors.top: verticalSpacer.bottom

                VPNSearchBar {
                    id: searchBar

                    _filterProxySource: VPNServerCountryModel
                    _filterProxyCallback: country => {
                            const searchString = getSearchBarText();
                            const includesSearchString = nameString => (
                                nameString.toLowerCase().includes(searchString)
                            );
                            const includesName = includesSearchString(country.name);
                            const includesLocalizedName = includesSearchString(country.localizedName);
                            const matchesCountryCode = country.code.toLowerCase() === searchString;

                            return includesName || includesLocalizedName || matchesCountryCode;
                        }
                    _searchBarHasError: () => { return countriesRepeater.count === 0 }
                    _searchBarPlaceholderText: VPNl18n.ServersViewSearchPlaceholder

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: VPNTheme.theme.vSpacing
                    anchors.rightMargin: VPNTheme.theme.vSpacing
                }


                VPNRecentConnections {
                    id: recentConnections
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: VPNTheme.theme.windowMargin / 2
                    anchors.leftMargin: anchors.rightMargin
                    visible: showRecentConnections && searchBar.getSearchBarText().length === 0
                    showMultiHopRecentConnections: false
                    height: showRecentConnections ? implicitHeight : 0
                }

                Repeater {
                    id: countriesRepeater
                    model: searchBar.getProxyModel()
                    delegate: VPNServerCountry{}
                }
            }
        }
    }

    Component {
        id: serverTabsComponent

        VPNTabNavigation {
            id: serverTabs

            anchors.fill: parent
            z: 1

            handleTabClick: (tabButton) => {
                if (tabButton.objectName === "tabAllServers") {
                    centerActiveServer(loaderServersAll.item);
                }
            }

            tabList: ListModel {
                id: tabButtonList

                ListElement {
                    tabLabelStringId: "ServersViewTabRecommendedLabel"
                    tabButtonId: "tabRecommendedServer"
                }

                ListElement {
                    tabLabelStringId: "ServersViewTabAllLabel"
                    tabButtonId: "tabAllServer"
                }
            }

            stackContent: [
                Loader {
                    id: loaderServersRecommended
                    sourceComponent: listServersRecommended
                },
                Loader {
                    id: loaderServersAll
                    sourceComponent: listServersAll
                }
            ]
        }
    }

    Loader {
        id: serverListLoader

        anchors.fill: parent
        sourceComponent: VPNFeatureList.get("recommendedServers").isSupported && showRecentConnections
            ? serverTabsComponent
            : listServersAll
        onStatusChanged: if (
            serverListLoader.status === Loader.Ready
            && showRecentConnections
        ) {
            centerActiveServer(serverListLoader.item);
        }
    }
}
