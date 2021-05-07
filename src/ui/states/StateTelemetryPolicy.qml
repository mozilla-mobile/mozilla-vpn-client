/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme
import "../components"

Item {
    Component.onCompleted: fade.start()

    VPNHeadline {
        id: headline

        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.08
        anchors.horizontalCenter: parent.horizontalCenter
        //% "TBD"
        text: qsTrId("vpn.telemetryPolicy..quickAccess")
    }

    VPNSubtitle {
        id: logoSubtitle

        //% "TBD"
        text: qsTrId("vpn.telemetryPolicy.statusBarIntro")
        anchors.top: headline.bottom
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
    }


    VPNButton {
        id: button
        objectName: "telemetryPolicyButton"

        //% "Continue"
        text: qsTrId("vpn.telemetryPolicy.continue")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        radius: 5
        onClicked: VPN.telemetryPolicyCompleted()
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 1000
    }

}
