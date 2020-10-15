/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme
import "../components"

Item {
    // TODO (can be taken from the settings view):
    // - manage account
    // - sign off

    Text {
        //% "Subscription needed"
        text: qsTrId("subscriptionNeeded")
    }

    VPNButton {
        width: 282
        //% "Subscribe now"
        text: qsTrId("subscribeNow")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        radius: 5
        onClicked: VPN.subscribe()
    }

}
