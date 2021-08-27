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
    property alias recentConnectionsList: recentConnectionsRepeater.model
    spacing: Theme.windowMargin / 2


    VPNBoldLabel {
        text: VPNl18n.tr(VPNl18n.MultiHopFeatureMultiHopConnectionsHeader)

        Layout.leftMargin: Theme.windowMargin
        Layout.minimumHeight: Theme.vSpacing
        verticalAlignment: Text.AlignVCenter
    }

    Repeater {
        id: recentConnectionsRepeater
        delegate: VPNClickableRow {

            // MULTIHOP TODO - Use real string
            accessibleName: "TODO"

            Layout.fillWidth: true
            Layout.preferredHeight: Theme.rowHeight
            anchors.left: undefined
            anchors.right: undefined
            anchors.leftMargin: undefined
            anchors.rightMargin: undefined

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

                    // MULTIHOP TODO - Use real list
                    serversList: model.servers
                }
            }
        }
    }

    Rectangle {
        Layout.preferredWidth: parent.width - Theme.windowMargin * 2
        Layout.preferredHeight: 1
        Layout.alignment: Qt.AlignHCenter
        color: Color.grey10
    }

}
