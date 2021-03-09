/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    property var headlineText
    property var errorMessage: ""
    property var errorMessage2: ""
    property var buttonText
    property var buttonObjectName
    property var buttonOnClick
    property var signOffLinkVisible: false
    property var getHelpLinkVisible: false
    id: vpnFlickable

    Component.onCompleted: {
        flickContentHeight = col.childrenRect.height + col.anchors.topMargin
    }

    ColumnLayout {
        id: col
        width: vpnFlickable.width
        anchors.top: parent.top
        anchors.topMargin: vpnFlickable.height * 0.08
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 32

        Component.onCompleted: {
            height = Math.max(window.height, childrenRect.height)
        }

        VPNHeadline {
            id: headline

            text: headlineText
            Layout.preferredHeight: paintedHeight
            Layout.preferredWidth: col.width - (Theme.windowMargin * 2)
            Layout.maximumWidth: 500
        }

        ColumnLayout {
            spacing: 24
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                id: warningIconWrapper

                Layout.preferredHeight: 48
                Layout.preferredWidth: 48
                Layout.alignment: Qt.AlignHCenter;
                color: Theme.red
                radius: height / 2

                Image {
                    source: "../resources/warning-white.svg"
                    antialiasing: true
                    sourceSize.height: 20
                    sourceSize.width: 20
                    anchors.centerIn: parent
                }
            }

            ColumnLayout {
                spacing: Theme.windowMargin
                Layout.alignment: Qt.AlignHCenter
                VPNTextBlock {
                    id: copyBlock1
                    Layout.preferredWidth: Theme.maxTextWidth
                    Layout.preferredHeight: paintedHeight
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: Theme.fontSize
                    lineHeight: 22
                    text: errorMessage
                }

                VPNTextBlock {
                    id: copyBlock2

                    Layout.preferredWidth: Theme.maxTextWidth
                    horizontalAlignment: Text.AlignHCenter
                    Layout.preferredHeight: paintedHeight
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: Theme.fontSize
                    lineHeight: 22
                    text: errorMessage2
                }
            }
        }

        ColumnLayout {
            spacing: Theme.windowMargin
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter


            VPNButton {
                id: btn

                objectName: buttonObjectName
                text: buttonText
                Layout.preferredHeight: Theme.rowHeight
                loaderVisible: false
                onClicked: buttonOnClick()
            }

            VPNFooterLink {
                id: getHelpLink

                visible: getHelpLinkVisible
                Layout.preferredHeight: Theme.rowHeight
                Layout.alignment: Qt.AlignHCenter
                labelText: qsTrId("vpn.main.getHelp")
                anchors.horizontalCenter: undefined
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                onClicked: stackview.push(getHelpComponent)
            }

            VPNSignOut {
                id: signOff

                visible: signOffLinkVisible
                Layout.preferredHeight: Theme.rowHeight
                Layout.alignment: Qt.AlignHCenter
                anchors.horizontalCenter: undefined
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                height: undefined
                onClicked: {
                    VPNController.logout();
                }
            }
        }

        Component {
            id: getHelpComponent

            VPNGetHelp {
                isSettingsView: false
            }
        }

        VPNVerticalSpacer {
            Layout.preferredHeight: Theme.windowMargin * 2
        }
    }
}
