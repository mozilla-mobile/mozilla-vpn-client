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
    Text {
        id: message

        //% "Subscription needed"
        text: qsTrId("vpn.iap.subscriptionNeeded")
    }

    VPNButton {
        id: manageAccountButton

        text: qsTrId("vpn.main.manageAccount")
        anchors.top: message.bottom
        anchors.topMargin: Theme.vSpacing
        onClicked: VPN.openLink(VPN.LinkAccount)
    }

    VPNButton {
        id: subscribeNow

        //% "Subscribe now"
        text: qsTrId("vpn.iap.subscribeNow")
        anchors.top: manageAccountButton.bottom
        anchors.topMargin: Theme.vSpacing
        radius: 5
        onClicked: VPN.subscribe()
    }

    VPNSignOut {
        id: signOutLink

        onClicked: VPNController.logout()
    }
}
