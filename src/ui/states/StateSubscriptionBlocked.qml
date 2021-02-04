/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: vpnFlickable

    Component.onCompleted: {
        flickContentHeight = vpnFlickable.childrenRect.height;
    }

    ColumnLayout {
        width: vpnFlickable.width
        anchors.fill: parent
        anchors.topMargin: vpnFlickable.height * 0.08

        VPNHeadline {
            id: headline

            //% "Error confirming subscription…"
            text: qsTrId("vpn.subscriptionBlocked.title")
            Layout.preferredHeight: paintedHeight
            Layout.preferredWidth: Math.max(Theme.maxTextWidth, vpnFlickable.width * 0.85)
        }


        Item {
            id: wrapper

            Layout.preferredHeight: parent.height - headline.paintedHeight - footerItems.childrenRect.height
            Layout.fillWidth: true
            Item {
                id: floatingContentWrapper

                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.left: parent.left
                height: childrenRect.height

                Rectangle {
                    id: warningIconWrapper

                    height: 48
                    width: 48
                    color: Theme.red
                    radius: height / 2
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        source: "../resources/warning-white.svg"
                        antialiasing: true
                        sourceSize.height: 20
                        sourceSize.width: 20
                        anchors.centerIn: parent
                    }

                }

                VPNTextBlock {
                    id: copyBlock1

                    anchors.top: warningIconWrapper.bottom
                    anchors.topMargin: Theme.windowMargin * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    width: Theme.maxTextWidth
                    font.pixelSize: Theme.fontSize
                    lineHeight: 22
                    //% "Another Firefox Account has already subscribed using this Apple ID."
                    text:qsTrId("vpn.subscriptionBlocked.anotherFxaSubscribed")
                }

                VPNTextBlock {
                    id: copyBlock2

                    anchors.top: copyBlock1.bottom
                    anchors.topMargin: Theme.windowMargin * 1.5
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    width: Theme.maxTextWidth
                    font.pixelSize: Theme.fontSize
                    lineHeight: 22
                    //% "Visit our help center to learn more about managing your subscriptions."
                    text: qsTrId("vpn.subscriptionBlocked.visitHelpCenter")
                }

                Rectangle {
                    height: Theme.rowHeight * 2
                    width: parent.width
                    color: "transparent"
                    anchors.top: copyBlock2.bottom
                }

            }

        }

        Item {
            id: footerItems
            Layout.preferredHeight: childrenRect.height
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom

            VPNButton {
                //% "Get Help"
                text: qsTrId("vpn.subscriptionBlocked.getHelp")
                anchors.bottom: spacer.top
                anchors.horizontalCenter: parent.horizontalCenter
                loaderVisible: false
                onClicked: VPN.openLink(VPN.LinkSubscriptionBlocked)
            }

            Rectangle {
                id: spacer
                anchors.bottom: signOff.top
                height: Theme.windowMargin
            }

            VPNSignOut {
                id: signOff

                height: Theme.rowHeight
                onClicked: {
                    VPNController.logout();
                }
            }

        }
    }
}
