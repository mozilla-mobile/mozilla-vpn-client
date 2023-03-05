/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    Component.onCompleted: fade.start()

    MZHeadline {
        id: headline

        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.08
        anchors.horizontalCenter: parent.horizontalCenter
        //% "Quick access"
        text: qsTrId("vpn.postAuthentication..quickAccess")
    }

    MZSubtitle {
        id: logoSubtitle

        //% "You can quickly access Mozilla VPN from your status bar."
        text: qsTrId("vpn.postAuthentication.statusBarIntro")
        anchors.top: headline.bottom
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
    }


    Image {
        source: "qrc:/ui/resources/quick-access.svg"
        sourceSize.height: 120
        sourceSize.width: 120
        anchors.centerIn: parent
    }

    MZButton {
        id: button
        objectName: "postAuthenticationButton"

        //% "Continue"
        text: qsTrId("vpn.postAuthentication.continue")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        radius: 5
        onClicked: VPN.postAuthenticationCompleted()
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 1000
    }

}
