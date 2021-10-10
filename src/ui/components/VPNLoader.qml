/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme
import "../components"

Item {
    id: root

    property var headlineText
    property var footerLinkIsVisible: true

    Component.onCompleted: fade.start()
    height: window.safeContentHeight
    width: parent.width

    VPNHeadline {
        id: headline

        anchors.top: root.top
        anchors.topMargin: root.height * 0.08
        anchors.horizontalCenter: root.horizontalCenter
        width: Math.min(Theme.maxTextWidth, root.width * .85)
        text: headlineText
    }


    Image {
        id: spinner

        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        sourceSize.height: 80
        fillMode: Image.PreserveAspectFit
        source: "../resources/spinner.svg"

        ParallelAnimation {
            id: startSpinning

            running: true

            PropertyAnimation {
                target: spinner
                property: "opacity"
                from: 0
                to: 1
                duration: 300
            }

            PropertyAnimation {
                target: spinner
                property: "scale"
                from: 0.7
                to: 1
                duration: 300
            }

            PropertyAnimation {
                target: spinner
                property: "rotation"
                from: 0
                to: 360
                duration: 8000
                loops: Animation.Infinite
            }

        }

    }

    VPNFooterLink {
        id: footerLink
        objectName: "cancelFooterLink"

        visible: footerLinkIsVisible
        //% "Cancel"
        labelText: qsTrId("vpn.authenticating.cancel")
        onClicked: VPN.cancelAuthentication()
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 1000
    }

}
