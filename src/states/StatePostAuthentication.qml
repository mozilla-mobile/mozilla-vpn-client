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
    FontLoader { id: boldest; source: "../resources/fonts/Metropolis-Bold.otf"; name: "MetropolisExtraBold";
        onStatusChanged: console.log(FontLoader.Ready, "IS IT???")
    }
    VPNHeadline {
        id: headline
        text: qsTr("Quick access")
        anchors.top: parent.top
        anchors.topMargin: 32
    }

    Text {
        id: logoSubtitle
        text: qsTr("You can quickly access Mozilla VPN from your status bar.")
        x: 169
        y: 255
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: headline.bottom
        anchors.topMargin: 8
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 15
        font.family: vpnFontInter.name
        wrapMode: Text.Wrap
        width: Theme.maxTextWidth
        color: Theme.fontColor
        lineHeightMode: Text.FixedHeight
        lineHeight: 22
    }

    Item {
        id: gradientText
        width: 213
        height: 131
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoSubtitle.bottom
        anchors.topMargin: 67

        Text {
            id: wip
            font.pixelSize: 58
            font.family: boldest.name
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.centerIn: gradientText
            anchors.fill: gradientText
            text: qsTr("WIP")
            visible: false
        }

        LinearGradient {
            anchors.fill: wip
            source: wip
            start: Qt.point(213, 0)
            end: Qt.point(0, 0)
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: "#FFA436"
                }
                GradientStop {
                    position: 0.31
                    color: "#FF4F5E"
                }
                GradientStop {
                    position: 0.66
                    color: "#FF298A"
                }
                GradientStop {
                    position: 1.0
                    color: "#9059FF"
                }
            }
        }
    }

    VPNButton {
        width: 282
        text: qsTr("Continue")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        radius: 5
        onClicked: VPN.postAuthenticationCompleted()
    }

    Component.onCompleted: fade.start()
    PropertyAnimation on opacity {
        id: fade
        from: 0
        to: 1
        duration: 1000
    }
}
