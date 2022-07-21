/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1
import components.forms 0.1

FocusScope {
    id: focusScope

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

        flickContentHeight: serverList.implicitHeight
        anchors.fill: parent


        NumberAnimation on contentY {
            id: scrollAnimation

            duration: 200
            easing.type: Easing.OutQuad
        }

        ColumnLayout {
            id: serverList
            objectName: "serverCountryList"

            spacing: 14
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top


            VPNSearchBar {
                id: serverSearchInput

                Layout.fillWidth: true
                Layout.leftMargin: VPNTheme.theme.vSpacing
                Layout.rightMargin: VPNTheme.theme.vSpacing
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                enabled: true
                onTextChanged: () => {
                    countriesModel.invalidate();
                }
                _placeholderText: VPNl18n.ServersViewSearchPlaceholder
                hasError: countriesRepeater.count === 0
                Keys.onDownPressed: recentConnections.visible ? recentConnections.focusItemAt(0) : countriesRepeater.itemAt(0).forceActiveFocus()
            }

            VPNFilterProxyModel {
                id: countriesModel
                source: VPNServerCountryModel
                filterCallback: country => {
                    const searchString = serverSearchInput.text.toLowerCase();
                    const includesSearchString = nameString => (
                        nameString.toLowerCase().includes(searchString)
                    );
                    const includesName = includesSearchString(country.name);
                    const includesLocalizedName = includesSearchString(country.localizedName);
                    const matchesCountryCode = country.code.toLowerCase() === searchString;

                    return includesName || includesLocalizedName || matchesCountryCode;
                }
            }

            VPNRecentConnections {
                id: recentConnections
                Layout.fillWidth: true
                Layout.rightMargin: VPNTheme.theme.windowMargin / 2
                Layout.leftMargin: VPNTheme.theme.windowMargin / 2
                Layout.preferredHeight: showRecentConnections ? implicitHeight : 0
                visible: showRecentConnections && serverSearchInput.text.length === 0
                showMultiHopRecentConnections: false
            }

            Repeater {
                id: countriesRepeater
                model: countriesModel
                delegate: VPNServerCountry{
                    //Necessary because VPNClickableRow has hardcoded anchors (that we aren't using here because this is a layout)
                    anchors.left: undefined
                    anchors.right: undefined
                    anchors.leftMargin: undefined
                    anchors.rightMargin: undefined

                    Layout.fillWidth: true
                    Layout.preferredHeight: height
                }
            }
        }
    }
}
