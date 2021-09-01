/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2
import Mozilla.VPN 1.0


import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color


ColumnLayout {
    property bool hasVisibleConnections: false
    property bool showMultiHopRecentConnections: true

    id: root

    spacing: Theme.windowMargin / 2
    visible: repeaterModel.count > 0

    function popStack() {
        stackview.pop()
    }

    VPNBoldLabel {
        id: recentConnectionsHeader
        text: VPNl18n.tr(VPNl18n.MultiHopFeatureMultiHopConnectionsHeader)
        Layout.leftMargin: Theme.windowMargin
        Layout.minimumHeight: Theme.vSpacing
        verticalAlignment: Text.AlignVCenter
        visible: repeaterModel.count > 0

    }

    ColumnLayout {
        id: connectionsCol

        spacing: Theme.windowMargin / 2
        Layout.fillWidth: true

        ListModel {
            property real maxNumVisibleConnections: 2
            id: repeaterModel

            Component.onCompleted: {
                // don't show the first/current entry
                for (let i=1; i<VPNSettings.recentConnections.length; i++) {

                    if (repeaterModel.count === maxNumVisibleConnections) {
                        return;
                    }

                    const recentConnection = VPNSettings.recentConnections[i];
                    const servers = recentConnection.split(" -> ");
                    const isMultiHop = servers.length > 1;

                    if (isMultiHop !== showMultiHopRecentConnections) {
                        return;
                    }

                    const connection = [];

                    for(let x = 0; x < servers.length; x++) {
                        const server = servers[x].split(",");
                        let serverCityName = server.slice(0, server.length -1)

                        // catch us cities with state abbreviations
                        // restyle the state string so that it can be sent in VPNController.changeServer
                        // TODO - make this less chancy - is there a way to grab the localized city name?
                        if (serverCityName.length > 1) {
                            let abbreviatedStateName = serverCityName[serverCityName.length - 1].replace(" ", "");
                            abbreviatedStateName = abbreviatedStateName.charAt(0).toUpperCase() + abbreviatedStateName.slice(1).toLowerCase();
                            serverCityName[serverCityName.length - 1] = abbreviatedStateName;
                            serverCityName = serverCityName.join(", ");
                        } else {
                            serverCityName = serverCityName.join("");
                        }

                        const countryCode = server[server.length - 1].split(" ").join("");

                        connection.push({
                             countryCode: countryCode,
                             localizedCityName: serverCityName
                         });
                    }

                    append({isMultiHop, connection });
                }
            }
        }

        Repeater {
            property real maxVisibleConnections: 2
            property real visibleConnections: 0
            id: recentConnectionsRepeater
            model: repeaterModel
            delegate: VPNClickableRow {
                id: del

                // MULTIHOP TODO - Use real string
                accessibleName: "TODO"

                Layout.fillWidth: true
                Layout.preferredHeight: Theme.rowHeight
                anchors.left: undefined
                anchors.right: undefined
                anchors.leftMargin: undefined
                anchors.rightMargin: undefined

                onClicked: {
                    let args = [];
                    popStack();

                    if (isMultiHop) {
                        return VPNController.changeServer(connection.get(1).countryCode, connection.get(1).localizedCityName, connection.get(0).countryCode, connection.get(0).localizedCityName)
                    }

                    return VPNController.changeServer(connection.get(0).countryCode, connection.get(0).localizedCityName)

                }

                RowLayout {
                    anchors.centerIn: parent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    width: parent.width - Theme.windowMargin * 2
                    height: parent.height

                    VPNServerLabel {
                        id: serverLabel
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        serversList: connection
                    }
                }
            }
        }
    }

    Rectangle {
        Layout.preferredWidth: parent.width - Theme.windowMargin * 2
        Layout.preferredHeight: 1
        Layout.alignment: Qt.AlignHCenter
        color: Color.grey10
        visible: repeaterModel.count > 0
    }

}
