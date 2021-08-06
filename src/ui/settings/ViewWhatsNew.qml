/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

import "../components"

Item {
    id: root
    VPNMenu {
        id: menu
        objectName: "settingsWhatsNew"

        title: "NeedString-What's new"
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        anchors.top: menu.bottom
        anchors.left: root.left
        anchors.right: root.right
        height: root.height - menu.height
        flickContentHeight: col.height

        ColumnLayout {
            id: col
            // TODO
            // Add UI Tour button
            // Add release notes copy
        }
    }
}
