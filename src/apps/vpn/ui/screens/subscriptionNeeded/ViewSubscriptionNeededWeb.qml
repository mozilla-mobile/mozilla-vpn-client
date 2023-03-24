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

    property bool wasmView: false

    flickContentHeight: Math.max(parent.height, headerLink.height + content.implicitHeight)
    height: parent.height
    interactive: flickContentHeight > height

    Rectangle {
        color: MZTheme.theme.bgColor
        anchors.fill: parent
    }

    MZHeaderLink {
        id: headerLink

        labelText: MZI18n.GetHelpLinkTitle
        onClicked: MZNavigator.requestScreen(VPN.ScreenGetHelp)
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
            Layout.topMargin: MZTheme.theme.vSpacing
        }

        MZHeadline {
            id: headline
            text: MZI18n.PurchaseWebTitle

            Layout.bottomMargin: MZTheme.theme.listSpacing
            Layout.topMargin: MZTheme.theme.vSpacingSmall
        }

        MZTextBlock {
            color: MZTheme.theme.fontColor
            font.pixelSize: MZTheme.theme.fontSize
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: MZI18n.PurchaseWebMessage
                .arg("<b style='color:" + MZTheme.theme.fontColorDark + ";'>"
                    + VPNUser.email + "</b>")
            wrapMode: Text.WordWrap
            width: Math.max(parent.width, MZTheme.theme.maxTextWidth)

            Layout.bottomMargin: MZTheme.theme.vSpacing * 2
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.vSpacingSmall
            Layout.rightMargin: MZTheme.theme.windowMargin

            Accessible.role: Accessible.StaticText
            // Prevent html tags from being read by screen readers
            Accessible.name: text.replace(/<[^>]*>/g, "")
        }

        MZButton {
            id: subscribeNow
            objectName: "vpnSubscriptionNeededView"
            text: MZI18n.PurchaseSubscribeNow

            onClicked: VPNPurchase.subscribe("web")

            Layout.bottomMargin: MZTheme.theme.vSpacingSmall
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.topMargin: MZTheme.theme.windowMargin * 0.75
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

            MZGreyLink {
                id: termsOfService

                labelText: MZI18n.AboutUsTermsOfService
                textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter

                onClicked: MZUrlOpener.openUrlLabel("termsOfService")

                Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
            }

            Rectangle {
                color: MZTheme.theme.greyLink.defaultColor
                height: MZTheme.theme.focusBorderWidth * 2
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.8
                radius: MZTheme.theme.focusBorderWidth
                visible: parent.flow !== Grid.TopToBottom
                width: height
            }

            MZGreyLink {
                id: privacyNotice
                labelText: MZI18n.AboutUsPrivacyNotice
                textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter

                onClicked: MZUrlOpener.openUrlLabel("privacyNotice")

                Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
            }
        }

        MZSignOut {
            anchors {
                bottom: undefined
                bottomMargin: undefined
                horizontalCenter: undefined
            }
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.topMargin: MZTheme.theme.vSpacing
        }

        MZVerticalSpacer {
            Layout.preferredHeight: MZTheme.theme.vSpacingSmall
            Layout.fillWidth: true
        }
    }
}
