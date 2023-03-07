/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

Rectangle {
    property bool isOpen: false

    anchors.fill: parent
    color: MZTheme.colors.primary
    radius: boxBackground.radius

    // IP Adresses
    ColumnLayout {
        anchors {
            left: parent.left
            leftMargin: MZTheme.theme.windowMargin * 1.5
            right: parent.right
            rightMargin: MZTheme.theme.windowMargin * 1.5
            top: parent.top
            topMargin: MZTheme.theme.windowMargin * 3
        }
        width: parent.width

        MZBoldLabel {
            color: MZTheme.colors.white
            text: MZI18n.ConnectionInfoMetricsTitle
            wrapMode: Text.WordWrap

            Accessible.role: Accessible.StaticText
            Accessible.name: text
            Layout.bottomMargin: MZTheme.theme.listSpacing * 1.5
            Layout.fillWidth: true
        }

        IPAddress {
            id: ipv4Address

            // Exit IP:
            ipVersionText: MZI18n.ConnectionInfoExitServerLabelIp
            ipAddressText: VPNIPAddressLookup.ipv4Address
            visible: VPNIPAddressLookup.ipv4Address !== ""
        }

        Rectangle {
            color: MZTheme.colors.white
            height: 1
            opacity: 0.2
            visible: ipv4Address.visible
            Layout.fillWidth: true
        }

        IPAddress {
            id: ipv6Address

            // Exit IPv6:
            ipVersionText: MZI18n.ConnectionInfoExitServerLabelIpv6
            ipAddressText: VPNIPAddressLookup.ipv6Address
            visible: VPNIPAddressLookup.ipv6Address !== ""
        }

        Rectangle {
            color: MZTheme.colors.white
            height: 1
            opacity: 0.2
            visible: ipv6Address.visible
            Layout.fillWidth: true
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }
}
