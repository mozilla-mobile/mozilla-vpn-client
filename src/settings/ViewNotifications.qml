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
        title: qsTr("Notifications")
        isSettingsView: true
    }

    ScrollBar.vertical: ScrollBar {}
}
