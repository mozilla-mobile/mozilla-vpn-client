/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import compat 0.1

FocusScope {
    id: focusScope

    property real listOffset: (MZTheme.theme.menuHeight * 2)
    property bool showRecentConnections: false
    property bool showRecommendedConnections: (showRecentConnections
        && MZFeatureList.get("recommendedServers").isSupported)
    property var currentServer

    function setSelectedServer(countryCode, cityName, localizedCityName) {
        Glean.sample.userChangedEndpointGeo.record({ server: currentServer.whichHop });

        if (currentServer.whichHop === "singleHopServer") {
            VPNCurrentServer.changeServer(countryCode, cityName);
            stackview.pop();
            return;
        }

        segmentedNav[currentServer.whichHop] = [countryCode, cityName, localizedCityName];
        multiHopStackView.pop();
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Accessible.name: menu.title
    Accessible.role: Accessible.List

    ButtonGroup {
        id: radioButtonGroup
    }

    // Recommended servers list
    Component {
        id: listServersRecommended

        MZFlickable {
            objectName: "serverCountryViewRecommend"
            id: vpnFlickable

            anchors.fill: parent
            flickContentHeight: serverListRecommended.implicitHeight + serverListRecommended.anchors.topMargin

            Column {
                id: serverListRecommended
                objectName: "serverListRecommended"

                spacing: MZTheme.theme.listSpacing * 1.5
                width: parent.width

                anchors {
                    top: parent.top
                    topMargin: MZTheme.theme.vSpacingSmall
                    left: parent.left
                    right: parent.right
                }

                MZCollapsibleCard {
                    anchors.horizontalCenter: parent.horizontalCenter

                    iconSrc: "qrc:/ui/resources/tip.svg"
                    contentItem: MZTextBlock {
                        text: MZI18n.ServersViewRecommendedCardBody
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                    }
                    title: MZI18n.ServersViewRecommendedCardTitle
                    width: parent.width - MZTheme.theme.windowMargin * 2
                }

                // Status component
                // TODO: Refresh server list and handle states
                MZClickableRow {
                    id: statusComponent

                    anchors {
                        leftMargin: MZTheme.theme.windowMargin * 0.5
                        rightMargin: MZTheme.theme.windowMargin * 0.5
                    }
                    accessibleName: MZI18n.ServersViewRecommendedRefreshLabel
                    canGrowVertical: true
                    height: statusTitle.implicitHeight + MZTheme.theme.vSpacingSmall
                    rowShouldBeDisabled: !(VPNController.state === VPNController.StateOff) || VPNServerLatency.isActive
                    opacity: 1.0

                    onClicked: {
                        VPNServerLatency.refresh();
                    }

                    RowLayout {
                        anchors {
                            fill: parent
                            leftMargin: MZTheme.theme.listSpacing
                            rightMargin: MZTheme.theme.listSpacing
                        }
                        spacing: MZTheme.theme.listSpacing

                        MZIcon {
                            source: "qrc:/nebula/resources/clock.svg"

                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.topMargin: MZTheme.theme.listSpacing
                        }

                        MZInterLabel {
                            id: statusTitle

                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            Layout.fillWidth: true
                            Layout.preferredWidth: parent.width
                            Layout.maximumWidth: parent.width

                            color: MZTheme.theme.fontColor
                            horizontalAlignment: Text.AlignLeft
                            // TODO: Replace placeholder strings and generate
                            // values that will be set instead of `%1`
                            text: VPNServerLatency.isActive
                                ? MZI18n.ServersViewRecommendedRefreshlLoadingLabel.arg(Math.round(VPNServerLatency.progress * 100))
                                : (VPNController.state === VPNController.StateOff)
                                ? MZI18n.ServersViewRecommendedRefreshLastUpdatedLabel.arg(MZLocalizer.formatDate(new Date(), VPNServerLatency.lastUpdateTime, MZI18n.ServersViewRecommendedRefreshLastUpdatedLabelYesterday))
                                : MZI18n.ServersViewRecommendedRefreshLastUpdatedDisabledLabel.arg(MZLocalizer.formatDate(new Date(), VPNServerLatency.lastUpdateTime, MZI18n.ServersViewRecommendedRefreshLastUpdatedLabelYesterday))
                            wrapMode: Text.WordWrap
                        }

                        Item {
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                            height: MZTheme.theme.iconSize * 1.5
                            visible: !statusComponent.rowShouldBeDisabled
                            width: MZTheme.theme.iconSize * 1.5

                            MZIcon {
                                id: refreshIcon
                                source: "qrc:/nebula/resources/refresh.svg"
                                sourceSize.height: parent.height
                                sourceSize.width: parent.width
                            }

                            MZColorOverlay {
                                anchors.fill: parent

                                color: MZTheme.colors.blue
                                source: refreshIcon
                            }
                        }
                    }
                }

                Repeater {
                    id: recommendedRepeater
                    model: VPNRecommendedLocationModel

                    delegate: MZClickableRow {
                        property bool isAvailable: modelData.connectionScore >= 0
                        id: recommendedServer

                        accessibleName: latencyIndicator.accessibleName.arg(modelData.localizedName)

                        onClicked: {
                            if (!isAvailable) {
                                return;
                            }
                            focusScope.setSelectedServer(city.country, city.name, city.localizedName);
                            MZSettings.recommendedServerSelected = true
                        }

                        RowLayout {
                            anchors.centerIn: parent;
                            height: parent.height
                            width: parent.width - MZTheme.theme.windowMargin

                            ServerLabel {
                                id: recommendedServerLabel

                                Layout.leftMargin: MZTheme.theme.listSpacing * 0.5
                                fontColor: MZTheme.theme.fontColorDark
                                narrowStyle: false
                                serversList: [{
                                    countryCode: city.country,
                                    cityName: city.name,
                                    localizedCityName: city.localizedName
                                }]
                            }

                            ServerLatencyIndicator {
                                id: latencyIndicator
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                score: city.connectionScore
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

        MZFlickable {
            objectName: "serverCountryView"
            id: vpnFlickable

            anchors.fill: parent

            Column {
                id: serverList
                objectName: "serverCountryList"

                spacing: MZTheme.theme.listSpacing * 1.75
                width: parent.width
                anchors.top: parent.top

                Item {
                    id: serverListSpacer
                    height: MZTheme.theme.vSpacing - parent.spacing
                    width: parent.width
                }

                MZSearchBar {
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
                    _searchBarHasError: countriesListView.count === 0
                    _searchBarPlaceholderText: MZI18n.ServersViewSearchPlaceholder

                    anchors {
                        left: parent.left
                        leftMargin: MZTheme.theme.vSpacing
                        right: parent.right
                        rightMargin: MZTheme.theme.vSpacing
                    }
                }

                Component {
                    id: countriesListViewHeader
                    RecentConnections {
                        id: recentConnections
                        anchors {
                            left: parent.left
                            leftMargin: anchors.rightMargin
                            right: parent.right
                            rightMargin: MZTheme.theme.windowMargin * 0.5
                        }
                        height: implicitHeight
                        showMultiHopRecentConnections: false
                    }
                }

                Component{
                    id: countriesListViewFooter
                    Item {
                        height: 2 * MZTheme.theme.menuHeight
                        width: parent.width
                    }
                }

                ListView {
                    id: countriesListView
                    model: searchBar.getProxyModel()
                    delegate: ServerCountry {}
                    clip: true
                    height: vpnFlickable.height - searchBar.height - serverListSpacer.height - serverList.spacing
                    width: parent.width
                    header: (showRecentConnections && searchBar.getSearchBarText().length === 0) ? countriesListViewHeader : Item
                    footer: countriesListViewFooter
                    Component.onCompleted: {
                        const index = VPNServerCountryModel.indexOfCountryCode(currentServer.countryCode);
                        positionViewAtIndex(index, ListView.Beginning);
                    }
                }
            }
        }
    }

    Component {
        id: serverTabsComponent

        MZTabNavigation {
            id: serverTabs

            height: parent.height - MZTheme.theme.menuHeight
            width: parent.width
            z: 1

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

            Component.onCompleted: {
                serverTabs.setCurrentTabIndex(MZSettings.recommendedServerSelected ? 0 : 1)
            }
        }
    }

    Loader {
        id: serverListLoader

        anchors.fill: parent
        sourceComponent: showRecommendedConnections
            ? serverTabsComponent
            : listServersAll
    }
}
