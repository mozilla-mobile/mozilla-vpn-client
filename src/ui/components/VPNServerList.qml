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


FocusScope {
    id: focusScope

    property real listOffset: (Theme.menuHeight * 2)
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

        flickContentHeight: serverList.implicitHeight + listOffset
        anchors.fill: parent

        Rectangle {
            id: verticalSpacer

            height: Theme.vSpacing
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

            VPNSearchBar {
                id: serverSearchInput

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.vSpacing
                anchors.rightMargin: Theme.vSpacing
                enabled: true
                height: Theme.rowHeight
                onTextChanged: () => {
                    countriesModel.invalidate();
                }
                placeholderText: VPNl18n.ServersViewSearchPlaceholder
                hasError: countriesRepeater.count === 0
                Keys.onDownPressed: recentConnections.visible ? recentConnections.focusItemAt(0) : countriesRepeater.itemAt(0).forceActiveFocus()

                VPNInputMessages {
                    anchors.top: serverSearchInput.bottom
                    anchors.topMargin: Theme.listSpacing

                    messages: [
                        {
                            type: "error",
                            message: VPNl18n.ServersViewSearchNoResultsLabel,
                            visible: serverSearchInput.hasError
                        }
                    ]
                }
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
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: Theme.windowMargin / 2
                anchors.leftMargin: anchors.rightMargin
                visible: showRecentConnections && serverSearchInput.text.length === 0
                showMultiHopRecentConnections: false
                height: showRecentConnections ? implicitHeight : 0
            }

            Repeater {
                id: countriesRepeater
                model: countriesModel
                delegate: VPNServerCountry{}
            }
        }
    }
}
