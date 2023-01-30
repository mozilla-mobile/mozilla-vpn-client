/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: root

    property string headlineText
    property bool footerLinkIsVisible: true

    Component.onCompleted: fade.start()
    height: window.safeContentHeight
    width: parent.width

    VPNHeadline {
        id: headline

        anchors.top: root.top
        anchors.topMargin: root.height * 0.08
        anchors.horizontalCenter: root.horizontalCenter
        width: Math.min(VPNTheme.theme.maxTextWidth, root.width * .85)
        text: headlineText
    }


    Image {
        id: spinner

        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        sourceSize.height: 80
        fillMode: Image.PreserveAspectFit
        source: "qrc:/nebula/resources/spinner.svg"

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

    VPNCancelButton {
        id: footerLink
        objectName: "cancelFooterLink"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.min(window.safeContentHeight * .08, 60)

        visible: footerLinkIsVisible
        onClicked: {
            if (typeof(isReauthFlow) !== "undefined" && isReauthFlow) {
                cancelAuthenticationFlow();
            } else {
                VPN.cancelAuthentication()
            }
        }
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 1000
    }

}
