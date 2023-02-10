/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2

import Mozilla.VPN 1.0
import compat 0.1
import components 0.1
import components.forms 0.1

FocusScope {
    id: focusScope

    property real listOffset: (VPNTheme.theme.menuHeight * 2)
    property bool showRecentConnections: false
    property bool showRecommendedConnections: (showRecentConnections
        && VPNFeatureList.get("recommendedServers").isSupported)
    property var currentServer

    function setSelectedServer(countryCode, cityName, localizedCityName) {
        if (currentServer.whichHop === "singleHopServer") {
            VPNCurrentServer.changeServer(countryCode, cityName);
            stackview.pop();
            return;
        }

        segmentedNav[currentServer.whichHop] = [countryCode, cityName, localizedCityName];
        multiHopStackView.pop();
    }

    function scrollToActiveServer(serverListFlickable) {
        if (!(serverListFlickable && serverListFlickable.countries)) {
            return;
        }
        // Scroll vpnFlickable so that the current server city is
        // vertically centered in the view
        const serverListYCenter = serverListFlickable.height * 0.5 - listOffset

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

    function centerActiveServer() {
        scrollToActiveServer(serverListLoader.item);
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Accessible.name: menu.title
    Accessible.role: Accessible.List

    ButtonGroup {
        id: radioButtonGroup
    }

    Component.onCompleted: {
        centerActiveServer();
    }

    // Recommended servers list
    Component {
        id: listServersRecommended

        VPNFlickable {
            objectName: "serverCountryViewRecommend"
            id: vpnFlickableRecommended

            anchors.fill: parent
            flickContentHeight: serverListRecommended.implicitHeight + listOffset

            Column {
                id: serverListRecommended
                spacing: VPNTheme.theme.listSpacing * 1.5
                width: parent.width

                anchors {
                    top: parent.top
                    topMargin: VPNTheme.theme.vSpacingSmall
                    left: parent.left
                    right: parent.right
                }

                VPNCollapsibleCard {
                    anchors.horizontalCenter: parent.horizontalCenter

                    iconSrc: "qrc:/ui/resources/tip.svg"
                    contentItem: VPNTextBlock {
                        text: VPNI18n.ServersViewRecommendedCardBody
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                    }
                    title: VPNI18n.ServersViewRecommendedCardTitle
                    width: parent.width - VPNTheme.theme.windowMargin * 2
                }

                // Status component
                // TODO: Refresh server list and handle states
                VPNClickableRow {
                    id: statusComponent

                    anchors {
                        leftMargin: VPNTheme.theme.windowMargin * 0.5
                        rightMargin: VPNTheme.theme.windowMargin * 0.5
                    }
                    accessibleName: VPNI18n.ServersViewRecommendedRefreshLabel
                    canGrowVertical: true
                    height: statusTitle.implicitHeight + VPNTheme.theme.vSpacingSmall
                    rowShouldBeDisabled: !(VPNController.state === VPNController.StateOff)

                    onClicked: {
                        console.log("TODO: Request server data refresh");
                    }

                    RowLayout {
                        anchors {
                            fill: parent
                            leftMargin: VPNTheme.theme.listSpacing
                            rightMargin: VPNTheme.theme.listSpacing
                        }
                        spacing: VPNTheme.theme.listSpacing

                        VPNIcon {
                            source: "qrc:/nebula/resources/clock.svg"

                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.topMargin: VPNTheme.theme.listSpacing
                        }

                        VPNInterLabel {
                            id: statusTitle

                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            Layout.fillWidth: true
                            Layout.preferredWidth: parent.width
                            Layout.maximumWidth: parent.width

                            color: VPNTheme.theme.fontColor
                            horizontalAlignment: Text.AlignLeft
                            // TODO: Replace placeholder strings and generate
                            // values that will be set instead of `%1`
                            text: !statusComponent.rowShouldBeDisabled
                                ? "Last updated %1 ago."
                                : "Last updated %1 ago. To update this list please first disconnect from the VPN."
                            wrapMode: Text.WordWrap
                        }

                        Item {
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                            height: VPNTheme.theme.iconSize * 1.5
                            visible: !statusComponent.rowShouldBeDisabled
                            width: VPNTheme.theme.iconSize * 1.5

                            VPNIcon {
                                id: refreshIcon
                                source: "qrc:/nebula/resources/refresh.svg"
                                sourceSize.height: parent.height
                                sourceSize.width: parent.width
                            }

                            VPNColorOverlay {
                                anchors.fill: parent

                                color: VPNTheme.colors.blue
                                source: refreshIcon
                            }
                        }
                    }
                }

                Repeater {
                    id: recommendedRepeater
                    model: VPNServerCountryModel.recommendedLocations(5)
                    delegate: VPNClickableRow {
                        property bool isAvailable: modelData.connectionScore >= 0
                        id: recommendedServer

                        accessibleName: modelData.localizedName
                        onClicked: {
                            if (!isAvailable) {
                                return;
                            }
                            focusScope.setSelectedServer(modelData.country, modelData.name, modelData.localizedName);
                        }

                        RowLayout {
                            anchors.centerIn: parent;
                            height: parent.height
                            width: parent.width - VPNTheme.theme.windowMargin

                            VPNServerLabel {
                                id: recommendedServerLabel

                                Layout.leftMargin: VPNTheme.theme.listSpacing * 0.5
                                fontColor: VPNTheme.theme.fontColorDark
                                narrowStyle: false
                                serversList: [{
                                    countryCode: modelData.country,
                                    cityName: modelData.name,
                                    localizedCityName: modelData.localizedName
                                }]
                            }

                            VPNServerLatencyIndicator {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                score: modelData.connectionScore
                            }
                        }
                    }
                }
            }
        }
    }

    // All servers list
    Component {
        id: listServersAll

        VPNFlickable {
            objectName: "serverCountryView"
            property alias countries: countriesRepeater
            id: vpnFlickable

            flickContentHeight: serverList.implicitHeight
            anchors.fill: parent

            NumberAnimation on contentY {
                id: scrollAnimation

                duration: 200
                easing.type: Easing.OutQuad
            }

            Column {
                id: serverList
                objectName: "serverCountryList"

                spacing: VPNTheme.theme.listSpacing * 1.75
                width: parent.width
                anchors.top: parent.top

                Item {
                    height: VPNTheme.theme.vSpacing - parent.spacing
                    width: parent.width
                }

                VPNSearchBar {
                    id: searchBar
                    objectName: "countrySearchBar"

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
                    _searchBarHasError: countriesRepeater.count === 0
                    _searchBarPlaceholderText: VPNI18n.ServersViewSearchPlaceholder

                    anchors {
                        left: parent.left
                        leftMargin: VPNTheme.theme.vSpacing
                        right: parent.right
                        rightMargin: VPNTheme.theme.vSpacing
                    }
                }


                VPNRecentConnections {
                    id: recentConnections
                    anchors {
                        left: parent.left
                        leftMargin: anchors.rightMargin
                        right: parent.right
                        rightMargin: VPNTheme.theme.windowMargin * 0.5
                    }
                    height: showRecentConnections ? implicitHeight : 0
                    showMultiHopRecentConnections: false
                    visible: showRecentConnections && searchBar.getSearchBarText().length === 0
                }

                Repeater {
                    id: countriesRepeater
                    model: searchBar.getProxyModel()
                    delegate: VPNServerCountry {}
                }
            }
        }
    }

    Component {
        id: serverTabsComponent

        VPNTabNavigation {
            id: serverTabs

            height: parent.height - VPNTheme.theme.menuHeight
            width: parent.width
            z: 1

            handleTabClick: (tabButton) => {
                if (tabButton.objectName === "tabAllServers") {
                    scrollToActiveServer(loaderServersAll.item);
                }
            }

            tabList: ListModel {
                id: tabButtonList

                ListElement {
                    tabLabelStringId: "ServersViewTabRecommendedLabel"
                    tabButtonId: "tabRecommendedServers"
                }

                ListElement {
                    tabLabelStringId: "ServersViewTabAllLabel"
                    tabButtonId: "tabAllServers"
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
        sourceComponent: showRecommendedConnections
            ? serverTabsComponent
            : listServersAll
        onStatusChanged: if (serverListLoader.status === Loader.Ready) {
            centerActiveServer();
        }
    }
}
