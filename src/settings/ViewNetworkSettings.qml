/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5

import Mozilla.VPN 1.0

import "../components"

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Network settings")
        isSettingsView: true
    }

    CheckBox {
        id: settingsIpv6
        checked: VPNSettings.ipv6
        text: qsTr("IPv6 enabled")
        onClicked: VPNSettings.ipv6 = checked
        y: menu.y + menu.height
    }

    CheckBox {
        checked: VPNSettings.localNetwork
        text: qsTr("Local network enabled")
        onClicked: VPNSettings.localNetwork = checked
        y: settingsIpv6.y + settingsIpv6.height
    }

    ScrollBar.vertical: ScrollBar {}
}
