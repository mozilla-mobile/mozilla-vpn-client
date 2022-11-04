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

    Component.onCompleted: centerActiveServer();

    function centerActiveServer() {

        // Scroll vpnFlickable so that the current server city is
        // vertically centered in the view
        const serverListYCenter = vpnFlickable.height / 2 - listOffset

        for (let idx = 0; idx < countriesRepeater.count; idx++) {
            const countryItem = countriesRepeater.itemAt(idx);

            if (
                // Country does not host current active server
                countryItem._countryCode !== currentServer.countryCode ||

                // Country is already above the vertical center
                countryItem.y < serverListYCenter
                ) {
                continue;
            }

            // Get distance to the current server city and scroll
            const currentCityYPosition = countryItem.y + (54 * countryItem.currentCityIndex) - serverListYCenter;
            const destinationY = (currentCityYPosition + vpnFlickable.height > vpnFlickable.contentHeight) ? vpnFlickable.contentHeight - vpnFlickable.height : currentCityYPosition;
            vpnFlickable.contentY = destinationY;

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

    VPNFlickable {
        id: vpnFlickable
        objectName: "serverCountryView"

        flickContentHeight: serverList.implicitHeight + verticalSpacer.height
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

            spacing: 14
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
                _searchBarHasError: countriesRepeater.count === 0
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
