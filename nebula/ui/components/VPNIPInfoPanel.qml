/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
    property bool isOpen: false

    anchors.fill: parent
    color: VPNTheme.colors.primary
    radius: boxBackground.radius

    // IP Adresses
    ColumnLayout {
        anchors {
            left: parent.left
            leftMargin: VPNTheme.theme.windowMargin * 1.5
            right: parent.right
            rightMargin: VPNTheme.theme.windowMargin * 1.5
            top: parent.top
            topMargin: VPNTheme.theme.windowMargin * 3
        }
        width: parent.width

        VPNBoldLabel {
            color: VPNTheme.colors.white
            text: VPNI18n.ConnectionInfoMetricsTitle
            wrapMode: Text.WordWrap

            Accessible.role: Accessible.StaticText
            Accessible.name: text
            Layout.bottomMargin: VPNTheme.theme.listSpacing * 1.5
            Layout.fillWidth: true
        }

        VPNIPAddress {
            id: ipv4Address

            // Exit IP:
            ipVersionText: VPNI18n.ConnectionInfoExitServerLabelIp
            ipAddressText: VPNIPAddressLookup.ipv4Address
            visible: VPNIPAddressLookup.ipv4Address !== ""
        }

        Rectangle {
            color: VPNTheme.colors.white
            height: 1
            opacity: 0.2
            visible: ipv4Address.visible
            Layout.fillWidth: true
        }

        VPNIPAddress {
            id: ipv6Address

            // Exit IPv6:
            ipVersionText: VPNI18n.ConnectionInfoExitServerLabelIpv6
            ipAddressText: VPNIPAddressLookup.ipv6Address
            visible: VPNIPAddressLookup.ipv6Address !== ""
        }

        Rectangle {
            color: VPNTheme.colors.white
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
