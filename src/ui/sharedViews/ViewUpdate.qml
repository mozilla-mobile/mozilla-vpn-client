/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZFlickable {
    id: vpnFlickable

    flickContentHeight: vpnPanel.height + alertWrapperBackground.height + footerContent.height + (MZTheme.theme.windowMargin * 4)
    state: VPN.state === VPN.StateUpdateRequired ? "required" : "recommended"
    states: [
        State {
            name: "recommended"

            PropertyChanges {
                target: vpnPanel
                //% "Update recommended"
                logoTitle: qsTrId("vpn.updates.updateRecomended")
                //% "Please update the app before you continue to use the VPN"
                logoSubtitle: qsTrId("vpn.updates.updateRecomended.description")
                logo: "qrc:/ui/resources/updateRecommended.svg"
            }

            PropertyChanges {
                target: signOff
                visible: false
            }

            PropertyChanges {
                target: footerLink
                onClicked: {
                    MZNavigator.requestPreviousScreen();
                }
            }

        },
        State {
            name: "required"

            PropertyChanges {
                target: vpnPanel
                //% "Update required"
                logoTitle: qsTrId("vpn.updates.updateRequired")
                //% "We detected and fixed a serious bug. You must update your app."
                logoSubtitle: qsTrId("vpn.updates.updateRequire.reason")
                logo: "qrc:/ui/resources/updateRequired.svg"
            }

            PropertyChanges {
                target: signOff
                visible: VPN.userAuthenticated
            }

            PropertyChanges {
                target: footerLink
                onClicked: {
                    MZUrlOpener.openUrlLabel("account");
                }
            }

        }
    ]

    Item {
        id: spacer1

        height: Math.max(MZTheme.theme.windowMargin * 2, ( window.safeContentHeight - flickContentHeight ) / 2)
        width: vpnFlickable.width
    }

    MZPanel {
        id: vpnPanel

        property var childRectHeight: vpnPanel.childrenRect.height

        anchors.top: spacer1.bottom
        width: Math.min(vpnFlickable.width - MZTheme.theme.windowMargin * 4, MZTheme.theme.maxHorizontalContentWidth)
        logoSize: 80
    }

    Item {
        id: spacer2

        anchors.top: vpnPanel.bottom
        height: Math.max(MZTheme.theme.windowMargin * 2, (window.safeContentHeight -flickContentHeight ) / 2)
        width: vpnFlickable.width
    }

    MZDropShadowWithStates {
        anchors.fill: alertWrapperBackground
        source: alertWrapperBackground
    }

    Rectangle {
        id: alertWrapperBackground

        anchors.fill: alertWrapper
        color: MZTheme.theme.white
        radius: 8
        anchors.topMargin: -MZTheme.theme.windowMargin
        anchors.bottomMargin: -MZTheme.theme.windowMargin
        anchors.leftMargin: -MZTheme.theme.windowMargin
        anchors.rightMargin: -MZTheme.theme.windowMargin
    }

    ColumnLayout {
        id: alertWrapper

        anchors.top: spacer2.bottom
        anchors.topMargin: MZTheme.theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(vpnFlickable.width - (MZTheme.theme.windowMargin * 4), MZTheme.theme.maxHorizontalContentWidth)

        RowLayout {
            id: insecureConnectionAlert

            Layout.minimumHeight: 40
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 16

            Image {
                source: "qrc:/ui/resources/connection-info-dark.svg"
                sourceSize.width: 20
                sourceSize.height: 20
                antialiasing: true
            }

            MZTextBlock {
                id: alertUpdateRecommendedText

                font.family: MZTheme.theme.fontInterFamily
                font.pixelSize: MZTheme.theme.fontSizeSmall
                color: MZTheme.theme.fontColorDark
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                //% "Your connection will not be secure while you update."
                text: qsTrId("vpn.updates.updateConnectionInsecureWarning")
            }

        }

    }

    ColumnLayout {
        id: footerContent

        anchors.top: alertWrapperBackground.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, MZTheme.theme.maxHorizontalContentWidth)
        spacing: MZTheme.theme.windowMargin * 1.25

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: MZTheme.theme.windowMargin / 2
            color: MZTheme.theme.transparent
        }

        MZButton {
            id: updateBtn

            //% "Update now"
            text: qsTrId("vpn.updates.updateNow")
            loaderVisible: VPN.updating
            radius: 4
            onClicked: VPN.update()
        }

        MZLinkButton {
            id: footerLink

            //% "Not now"
            readonly property var textNotNow: qsTrId("vpn.updates.notNow")
            readonly property var textManageAccount: MZI18n.SubscriptionManagementManageAccount

            Layout.alignment: Qt.AlignHCenter
            labelText: (vpnFlickable.state === "recommended") ? textNotNow : textManageAccount
        }

        MZSignOut {
            id: signOff

            anchors.bottom: undefined
            anchors.bottomMargin: undefined
            anchors.horizontalCenter: undefined
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: MZTheme.theme.windowMargin * 2
            color: MZTheme.theme.transparent
        }

    }

    Component.onCompleted: VPN.updateViewShown()

}
