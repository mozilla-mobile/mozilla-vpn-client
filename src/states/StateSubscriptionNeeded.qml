/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme
import "../components"

Item {
    Text {
        text: qsTr("Subscription needed")
    }

    VPNButton {
        width: 282
        text: qsTr("Subscribe now")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        radius: 5
        onClicked: VPN.subscribe()
    }

    // TODO (can be taken from the settings view):
    // - manage account
    // - sign off
}
