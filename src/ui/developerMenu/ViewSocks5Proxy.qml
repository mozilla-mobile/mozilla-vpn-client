/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1
import components.forms 0.1

Item {
    id: root

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Proxy - REMOVE ME"
    }

    VPNFlickable {
        id: vpnFlickable
        flickContentHeight: proxyssHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: VPNTheme.theme.windowMargin

            spacing: VPNTheme.theme.windowMargin
            id: proxyssHolder

            VPNCheckBoxRow {
                showDivider: false
                labelText: "Enabled"
                subLabelText: "Socks5 proxy on"
                showAppImage: false
                enabled: true
                isChecked: VPNSettings.socks5ProxyEnabled;
                Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5

                onClicked: {
                   VPNSettings.socks5ProxyEnabled = !VPNSettings.socks5ProxyEnabled;
                }
            }

            Text {
                text: "Current connection: " + VPNSocks5Proxy.connections
            }

            // TODO: VPNSettings.socks5ProxyPort
        }
    }
}

