/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtWebView 1.15

import Mozilla.VPN 1.0
import components 0.1

Item {
    Item {
        id: menuBar

        width: parent.width
        height: 56
        // Ensure that menu is on top of possible scrollable
        // content.
        z: 1

        Rectangle {
            id: menuBackground
            color: VPNTheme.theme.bgColor
            y: 0
            width: parent.width
            height: 55
        }

        VPNIconButton {
            id: iconButton

            onClicked: {
                VPNAndroidUtils.abortAuthentication();
                authWebview.clearStorage();
                mainStackView.pop(StackView.Immediate);
            }

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: VPNTheme.theme.windowMargin / 2
            anchors.leftMargin: VPNTheme.theme.windowMargin / 2

            accessibleName: qsTrId("vpn.main.back")

            Image {
                id: backImage

                source: "qrc:/nebula/resources/close-dark.svg"
                sourceSize.width: VPNTheme.theme.iconSize
                fillMode: Image.PreserveAspectFit
                anchors.centerIn: iconButton
            }
        }

        VPNBoldLabel {
            id: title

            anchors.top: menuBar.top
            anchors.centerIn: menuBar

            //% "Authentication"
            text: qsTrId("vpn.android.authentication")
        }

        Rectangle {
            color: "#0C0C0D0A"
            y: 55
            width: parent.width
            height: 1
        }
    }

    VPNAndroidWebView {
        id: authWebview
        url: VPNAndroidUtils.url

        height: parent.height - menuBar.height
        width: parent.width
        y: menuBar.height

        onPageStarted: {
            if (VPNAndroidUtils.maybeCompleteAuthentication(url)) {
                mainStackView.pop(StackView.Immediate);
            }
        }

        onFailure: {
            VPNAndroidUtils.abortAuthentication();
            mainStackView.pop(StackView.Immediate)
        }
    }

    Component.onCompleted: VPNCloseEventHandler.addView(menuBar)

    Connections {
        target: VPNCloseEventHandler
        function onGoBack(item) {
            if (item === menuBar) {
                VPNAndroidUtils.abortAuthentication();
                mainStackView.pop(StackView.Immediate);
            }
        }
    }
}
