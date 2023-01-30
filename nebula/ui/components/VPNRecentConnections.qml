/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0

ColumnLayout {
    id: root

    property bool hasVisibleConnections: false
    property bool showMultiHopRecentConnections: true
    property real numVisibleConnections: recentConnectionsRepeater.count
    property var recentConnectionsModel: showMultiHopRecentConnections ? VPNRecentConnectionsModel.multiHopModel : VPNRecentConnectionsModel.singleHopModel

    function focusItemAt(idx) {
        if (!visible) {
            return serverSearchInput.forceActiveFocus()
        }

        recentConnectionsRepeater.itemAt(idx).forceActiveFocus();
    }

    function generateServersList(exitCountryCode, localizedExitCityName, isMultiHop, entryCountryCode, localizedEntryCityName) {
        const list = [];
        if (isMultiHop) {
            list.push({
                countryCode: entryCountryCode,
                localizedCityName: localizedEntryCityName,
            });
        }
        list.push({
            countryCode: exitCountryCode,
            localizedCityName: localizedExitCityName,
        });

        return list;
    }

    spacing: VPNTheme.theme.windowMargin / 2
    visible: !root.recentConnectionsModel.isEmpty

    function popStack() {
        stackview.pop()
    }

    VPNBoldLabel {
        text: VPNl18n.MultiHopFeatureMultiHopConnectionsHeader
        Layout.leftMargin: VPNTheme.theme.windowMargin
        Layout.minimumHeight: VPNTheme.theme.vSpacing
        verticalAlignment: Text.AlignVCenter
        visible: !root.recentConnectionsModel.isEmpty

    }

    ColumnLayout {
        spacing: VPNTheme.theme.windowMargin / 2
        Layout.fillWidth: true

        Repeater {
            property real maxVisibleConnections: 2
            property real visibleConnections: 0
            id: recentConnectionsRepeater
            model: root.recentConnectionsModel
            delegate: VPNClickableRow {
                accessibleName: isMultiHop ?
                    VPNl18n.MultiHopFeatureAccessibleNameRecentConnection
                        .arg(localizedEntryCityName).arg(localizedExitCityName)
                    : localizedExitCityName;

                Layout.fillWidth: true
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                anchors.left: undefined
                anchors.right: undefined
                anchors.leftMargin: undefined
                anchors.rightMargin: undefined

                Keys.onDownPressed: recentConnectionsRepeater.itemAt(index + 1) ? recentConnectionsRepeater.itemAt(index + 1).forceActiveFocus() : countriesRepeater.itemAt(0).forceActiveFocus()
                Keys.onUpPressed: recentConnectionsRepeater.itemAt(index - 1) ? recentConnectionsRepeater.itemAt(index - 1).forceActiveFocus() : serverSearchInput.forceActiveFocus()

                onClicked: {
                    popStack();
                    VPNCurrentServer.changeServer(exitCountryCode, exitCityName, entryCountryCode, entryCityName);
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
                        serversList: generateServersList(exitCountryCode, localizedExitCityName, isMultiHop, entryCountryCode, localizedEntryCityName);
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
        visible: !root.recentConnectionsModel.isEmpty
    }

}
