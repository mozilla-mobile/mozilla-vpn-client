/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNFlickable {
    id: vpnFlickable

    property bool wasmView: false

    flickContentHeight: Math.max(parent.height, headerLink.height + content.implicitHeight)
    height: parent.height
    interactive: flickContentHeight > height

    Rectangle {
        color: VPNTheme.theme.bgColor
        anchors.fill: parent
    }

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp2")
        onClicked: VPNNavigator.requestScreen(VPNNavigator.ScreenGetHelp)
    }

    ColumnLayout {
        id: content

        anchors.verticalCenter: parent.verticalCenter
        width: parent.width

        Image {
            id: logo
            antialiasing: true
            smooth: true
            source: "qrc:/ui/resources/logo-connecting.svg"

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: VPNTheme.theme.vSpacing
        }

        VPNHeadline {
            id: headline
            text: VPNI18n.PurchaseWebTitle

            Layout.bottomMargin: VPNTheme.theme.listSpacing
            Layout.topMargin: VPNTheme.theme.vSpacingSmall
        }

        VPNTextBlock {
            color: VPNTheme.theme.fontColor
            font.pixelSize: VPNTheme.theme.fontSize
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: VPNI18n.PurchaseWebMessage
                .arg("<b style='color:" + VPNTheme.theme.fontColorDark + ";'>"
                    + VPNUser.email + "</b>")
            wrapMode: Text.WordWrap
            width: Math.max(parent.width, VPNTheme.theme.maxTextWidth)

            Layout.bottomMargin: VPNTheme.theme.vSpacing * 2
            Layout.fillWidth: true
            Layout.leftMargin: VPNTheme.theme.vSpacingSmall
            Layout.rightMargin: VPNTheme.theme.windowMargin

            Accessible.role: Accessible.StaticText
            // Prevent html tags from being read by screen readers
            Accessible.name: text.replace(/<[^>]*>/g, "")
        }

        VPNButton {
            id: subscribeNow
            objectName: "vpnSubscriptionNeededView"
            //% "Subscribe now"
            text: qsTrId("vpn.updates.subscribeNow")

            onClicked: VPNPurchase.subscribe("web")

            Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
            Layout.fillWidth: true
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.topMargin: VPNTheme.theme.windowMargin * 0.75
        }

        GridLayout {
            id: grid

            columnSpacing: 0
            columns: 3

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            Component.onCompleted: {
                if (implicitWidth > window.width) {
                    flow = Grid.TopToBottom
                }
            }

            VPNGreyLink {
                id: termsOfService

                // Terms of Service - string defined in ViewAboutUs.qml
                labelText: qsTrId("vpn.aboutUs.tos2")
                textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter

                onClicked: VPNUrlOpener.openUrlLabel("termsOfService")

                Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
            }

            Rectangle {
                color: VPNTheme.theme.greyLink.defaultColor
                height: VPNTheme.theme.focusBorderWidth * 2
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.8
                radius: VPNTheme.theme.focusBorderWidth
                visible: parent.flow !== Grid.TopToBottom
                width: height
            }

            VPNGreyLink {
                id: privacyNotice
                // Privacy Notice - string defined in ViewAboutUs.qml
                labelText: qsTrId("vpn.aboutUs.privacyNotice2")
                textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter

                onClicked: VPNUrlOpener.openUrlLabel("privacyNotice")

                Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
            }
        }

        VPNSignOut {
            anchors {
                bottom: undefined
                bottomMargin: undefined
                horizontalCenter: undefined
            }
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.topMargin: VPNTheme.theme.vSpacing
        }

        VPNVerticalSpacer {
            Layout.preferredHeight: VPNTheme.theme.vSpacingSmall
            Layout.fillWidth: true
        }
    }
}
