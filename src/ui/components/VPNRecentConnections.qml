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
    property real numVisibleConnections: recentConnectionsRepeater.count

    function focusItemAt(idx) {
        if (!visible) {
            return serverSearchInput.forceActiveFocus()
        }

        recentConnectionsRepeater.itemAt(idx).forceActiveFocus();
    }


    id: root

    spacing: Theme.windowMargin / 2
    visible: repeaterModel.count > 0

    function popStack() {
        stackview.pop()
    }

    VPNBoldLabel {
        id: recentConnectionsHeader
        text: VPNl18n.MultiHopFeatureMultiHopConnectionsHeader
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
                        let index = servers[x].lastIndexOf(",");
                        if (index <= 0) {
                            console.log("Unable to parse server from " + servers[x]);
                            continue;
                        }
                        let countryCode = servers[x].slice(index+1).trim();
                        let serverCityName = servers[x].slice(0, index).trim();

                        connection.push({
                             countryCode: countryCode,
                             serverCityName: serverCityName,
                             localizedCityName: VPNLocalizer.localizedCityName(countryCode, serverCityName)
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

                Keys.onDownPressed: recentConnectionsRepeater.itemAt(index + 1) ? recentConnectionsRepeater.itemAt(index + 1).forceActiveFocus() : countriesRepeater.itemAt(0).forceActiveFocus()
                Keys.onUpPressed: recentConnectionsRepeater.itemAt(index - 1) ? recentConnectionsRepeater.itemAt(index - 1).forceActiveFocus() : serverSearchInput.forceActiveFocus()

                onClicked: {
                    let args = [];
                    popStack();

                    if (isMultiHop) {
                        return VPNController.changeServer(connection.get(1).countryCode, connection.get(1).serverCityName, connection.get(0).countryCode, connection.get(0).serverCityName)
                    }

                    return VPNController.changeServer(connection.get(0).countryCode, connection.get(0).serverCityName)

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
