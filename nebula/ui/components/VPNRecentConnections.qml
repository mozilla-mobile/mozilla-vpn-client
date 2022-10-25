/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2

import Mozilla.VPN 1.0

ColumnLayout {
    property bool hasVisibleConnections: false
    property bool showMultiHopRecentConnections: true
    property real numVisibleConnections: recentConnectionsRepeater.count
    property var recentConnectionsModel: getRecentConnectionsModel()

    function getRecentConnectionsModel() {
        const maxNumVisibleConnections = 2
        const recentConnections = []
        for (let i=1; i<VPNSettings.recentConnections.length; i++) {
            if (recentConnections.length === maxNumVisibleConnections) {
                return recentConnections;
            }

            const recentConnection = VPNSettings.recentConnections[i];
            const servers = recentConnection.split(" -> ");
            const isMultiHop = servers.length > 1;

            if (isMultiHop !== showMultiHopRecentConnections) {
                return recentConnections;
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

            const [{ localizedCityName: firstCityLocalizedName }, secondServer] = connection;
            const accessibleLabel = secondServer
                ? VPNl18n.MultiHopFeatureAccessibleNameRecentConnection
                    .arg(firstCityLocalizedName)
                    .arg(secondServer.localizedCityName)
                : firstCityLocalizedName;

            recentConnections.push({
                isMultiHop,
                connection,
                accessibleLabel
            });
        }
        return recentConnections
    }

    function focusItemAt(idx) {
        if (!visible) {
            return serverSearchInput.forceActiveFocus()
        }

        recentConnectionsRepeater.itemAt(idx).forceActiveFocus();
    }


    id: root

    spacing: VPNTheme.theme.windowMargin / 2
    visible: root.recentConnectionsModel.length > 0

    function popStack() {
        stackview.pop()
    }

    VPNBoldLabel {
        id: recentConnectionsHeader
        text: VPNl18n.MultiHopFeatureMultiHopConnectionsHeader
        Layout.leftMargin: VPNTheme.theme.windowMargin
        Layout.minimumHeight: VPNTheme.theme.vSpacing
        verticalAlignment: Text.AlignVCenter
        visible: root.recentConnectionsModel.length > 0

    }

    ColumnLayout {
        id: connectionsCol

        spacing: VPNTheme.theme.windowMargin / 2
        Layout.fillWidth: true

        Repeater {
            property real maxVisibleConnections: 2
            property real visibleConnections: 0
            id: recentConnectionsRepeater
            model: root.recentConnectionsModel
            delegate: VPNClickableRow {
                id: del

                accessibleName: modelData.accessibleLabel

                Layout.fillWidth: true
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                anchors.left: undefined
                anchors.right: undefined
                anchors.leftMargin: undefined
                anchors.rightMargin: undefined

                Keys.onDownPressed: recentConnectionsRepeater.itemAt(index + 1) ? recentConnectionsRepeater.itemAt(index + 1).forceActiveFocus() : countriesRepeater.itemAt(0).forceActiveFocus()
                Keys.onUpPressed: recentConnectionsRepeater.itemAt(index - 1) ? recentConnectionsRepeater.itemAt(index - 1).forceActiveFocus() : serverSearchInput.forceActiveFocus()

                onClicked: {
                    let args = [];
                    popStack();

                    if (modelData.isMultiHop) {
                        return VPNController.changeServer(modelData.connection[1].countryCode, modelData.connection[1].serverCityName, modelData.connection[0].countryCode, modelData.connection[0].serverCityName)
                    }

                    return VPNController.changeServer(modelData.connection[0].countryCode, modelData.connection[0].serverCityName)

                }

                RowLayout {
                    anchors.centerIn: parent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    width: parent.width - VPNTheme.theme.windowMargin * 2
                    height: parent.height

                    VPNServerLabel {
                        id: serverLabel
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        serversList: modelData.connection
                    }
                }
            }
        }
    }

    Rectangle {
        Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin * 2
        Layout.preferredHeight: 1
        Layout.alignment: Qt.AlignHCenter
        color: VPNTheme.colors.grey10
        visible: root.recentConnectionsModel.length > 0
    }

}
