/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

Item {
    Component.onCompleted: fade.start()

    MZHeadline {
        id: headline

        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.08
        anchors.horizontalCenter: parent.horizontalCenter
        text: MZI18n.PostAuthenticationHeadline
    }

    MZSubtitle {
        id: logoSubtitle

        text: MZI18n.PostAuthenticationSubtitle
        anchors.top: headline.bottom
        anchors.left: parent.left
        anchors.right: parent.right
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

        text: MZI18n.GlobalContinue
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
