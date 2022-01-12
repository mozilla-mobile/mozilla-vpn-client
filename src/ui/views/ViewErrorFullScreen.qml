/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

VPNFlickable {
    property var headlineText
    property var errorMessage: ""
    property var errorMessage2: ""

    property var primaryButtonText
    property var primaryButtonObjectName
    property var primaryButtonOnClick

    property var secondaryButtonText: ""
    property var secondaryButtonObjectName
    property var secondaryButtonOnClick
    property var secondaryButtonIsSignOff: false

    property var getHelpLinkVisible: false
    property var statusLinkVisible: false
    id: vpnFlickable

    Component.onCompleted: {
        flickContentHeight = col.childrenRect.height
    }

    VPNHeaderLink {
        id: headerLink
        objectName: "getHelpLink"
        visible: getHelpLinkVisible

        labelText: qsTrId("vpn.main.getHelp2")
        onClicked: stackview.push("qrc:/ui/views/ViewGetHelp.qml", {isSettingsView: false})
    }

    ColumnLayout {
        id: col
        width: Math.min(VPNTheme.theme.maxHorizontalContentWidth, vpnFlickable.width)
        anchors.top: parent.top
        anchors.topMargin: headerLink.height + vpnFlickable.height * 0.08
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 32

        Component.onCompleted: {
            height = Math.max(window.height, childrenRect.height)
        }

        VPNHeadline {
            id: headline

            text: headlineText
            Layout.preferredHeight: paintedHeight
            Layout.preferredWidth: col.width - (VPNTheme.theme.windowMargin * 2)
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
                color: VPNTheme.theme.red
                radius: height / 2

                Image {
                    source: "qrc:/ui/resources/warning-white.svg"
                    antialiasing: true
                    sourceSize.height: 20
                    sourceSize.width: 20
                    anchors.centerIn: parent
                }
            }

            ColumnLayout {
                spacing: VPNTheme.theme.windowMargin
                Layout.alignment: Qt.AlignHCenter
                VPNTextBlock {
                    id: copyBlock1
                    Layout.preferredWidth: col.width - (VPNTheme.theme.windowMargin * 3)
                    Layout.preferredHeight: paintedHeight
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: VPNTheme.theme.fontSize
                    lineHeight: 22
                    text: errorMessage
                }

                VPNTextBlock {
                    id: copyBlock2

                    Layout.preferredWidth: col.width - (VPNTheme.theme.windowMargin * 3)
                    horizontalAlignment: Text.AlignHCenter
                    Layout.preferredHeight: paintedHeight
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: VPNTheme.theme.fontSize
                    lineHeight: 22
                    text: errorMessage2
                }

                VPNLinkButton {
                    //% "Check outage updates"
                    labelText: qsTrId("vpn.errors.checkOutageUpdates")
                    Layout.preferredWidth: col.width - (VPNTheme.theme.windowMargin * 3)
                    onClicked: VPN.openLink("https://status.vpn.mozilla.org")
                    Layout.alignment: Qt.AlignHCenter
                    visible: statusLinkVisible
                }
            }
        }

        ColumnLayout {
            spacing: VPNTheme.theme.windowMargin
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter


            VPNButton {
                id: primaryButton

                objectName: primaryButtonObjectName
                text: primaryButtonText
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                loaderVisible: false
                onClicked: primaryButtonOnClick()
            }
            
            VPNFooterLink {
                id: secondaryButton

                objectName: secondaryButtonObjectName
                labelText: secondaryButtonText
                visible: secondaryButtonText != "" && !secondaryButtonIsSignOff
                Layout.preferredHeight: Theme.rowHeight
                Layout.alignment: Qt.AlignHCenter
                anchors.horizontalCenter: undefined
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                height: undefined
                onClicked: secondaryButtonOnClick()
            }

            VPNSignOut {
                id: signOff

                visible: secondaryButtonIsSignOff
                Layout.preferredHeight: Theme.rowHeight
                Layout.alignment: Qt.AlignHCenter
                anchors.horizontalCenter: undefined
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                height: undefined
            }
        }


        VPNVerticalSpacer {
            Layout.preferredHeight: fullscreenRequired() ? VPNTheme.theme.windowMargin : 1
        }
    }
}
