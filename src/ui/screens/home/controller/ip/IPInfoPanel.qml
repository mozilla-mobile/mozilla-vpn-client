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
    property bool ready: !opacityAnimation.running

    readonly property bool showDetailedInfo: MZFeatureList.get("showDetailedConnectionInfo").isSupported

    anchors.fill: parent
    // Panel is fixed height, prefer clipping over overflow.
    clip: true
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
            topMargin: MZTheme.theme.windowMargin * (showDetailedInfo ? 1.5 : 3)
        }
        // Reduce the spacing when detailed infos are shown.
        spacing: MZTheme.theme.listSpacing / (showDetailedInfo ? 4 : 2)

        MZBoldLabel {
            color: MZTheme.colors.fontColorInverted
            text: MZI18n.ConnectionInfoMetricsTitle
            wrapMode: Text.WordWrap

            Layout.bottomMargin: MZTheme.theme.listSpacing * 1.5
            Layout.fillWidth: true
        }

        IPAddress {
            id: ipv4Address

            // Exit IP:
            ipVersionText: MZI18n.ConnectionInfoExitServerLabelIp
            ipAddressText: VPNIPAddressLookup.ipv4Address !== "" ? VPNIPAddressLookup.ipv4Address : MZI18n.ConnectionInfoLoading
            // Show when it has an IP address OR when neither have an IP address (and we default to showing "Loading")
            visible: VPNIPAddressLookup.ipv4Address !== "" || VPNIPAddressLookup.ipv6Address === ""
        }

        Rectangle {
            color: MZTheme.colors.fontColorInverted
            height: 1
            opacity: 0.2
            visible: ipv4Address.visible
            Layout.fillWidth: true
        }

        IPAddress {
            id: ipv6Address

            // Exit IPv6:
            ipVersionText: MZI18n.ConnectionInfoExitServerLabelIpv6
            ipAddressText: VPNIPAddressLookup.ipv6Address !== "" ? VPNIPAddressLookup.ipv6Address : MZI18n.ConnectionInfoLoading
            // Show when it has an IP address OR when neither have an IP address (and we default to showing "Loading")
            visible: VPNIPAddressLookup.ipv6Address !== "" || VPNIPAddressLookup.ipv4Address === ""
        }

        Rectangle {
            color: MZTheme.colors.fontColorInverted
            height: 1
            opacity: 0.2
            visible: ipv6Address.visible
            Layout.fillWidth: true
        }

        // The addresses this connection dials
        IPAddress {
            ipAddressText: VPNCurrentServer.entryServerIpv4Address
            ipVersionText: MZI18n.ConnectionInfoEntryEndpointLabelIp
            showCopyButton: true
            stackAddress: true
            showRotateButton: false
            visible: showDetailedInfo && ipAddressText !== ""
            Layout.topMargin: MZTheme.theme.listSpacing / 2
        }

        IPAddress {
            ipAddressText: VPNCurrentServer.entryServerIpv6Address
            ipVersionText: MZI18n.ConnectionInfoEntryEndpointLabelIpv6
            showCopyButton: true
            stackAddress: true
            showRotateButton: false
            visible: showDetailedInfo && ipAddressText !== ""
        }

        IPAddress {
            ipAddressText: VPNCurrentServer.exitServerIpv4Address
            ipVersionText: MZI18n.ConnectionInfoExitEndpointLabelIp
            showCopyButton: true
            stackAddress: true
            showRotateButton: false
            visible: showDetailedInfo && ipAddressText !== ""
            Layout.topMargin: MZTheme.theme.listSpacing / 2
        }

        IPAddress {
            ipAddressText: VPNCurrentServer.exitServerIpv6Address
            ipVersionText: MZI18n.ConnectionInfoExitEndpointLabelIpv6
            showCopyButton: true
            stackAddress: true
            showRotateButton: false
            visible: showDetailedInfo && ipAddressText !== ""
        }
    }

    Behavior on opacity {
        NumberAnimation {
            id: opacityAnimation
            duration: 300
        }
    }
}
