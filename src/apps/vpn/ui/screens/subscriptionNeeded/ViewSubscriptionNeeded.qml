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
    readonly property bool isMobile: Qt.platform.os === "android" || Qt.platform.os === "ios"

    flickContentHeight: headerLink.implicitHeight + col.implicitHeight + col.anchors.topMargin

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
        id: col

        anchors.top: headerLink.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 0

        Image {
            id: logo

            Layout.topMargin: isMobile ? MZTheme.theme.vSpacing : window.height * 0.2
            Layout.alignment: Qt.AlignHCenter

            source: "qrc:/ui/resources/logo-connecting.svg"
            smooth: true
            antialiasing: true
        }

        MZHeadline {
            id: headline

            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

            text: MZI18n.PurchaseWebTitle
        }

        MZTextBlock {
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

            text: MZI18n.PurchaseWebMessage
                .arg("<b style='color:" + MZTheme.theme.fontColorDark + ";'>"
                + VPNUser.email + "</b>")
            font.pixelSize: MZTheme.theme.fontSize
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            lineHeight: 22

            Accessible.role: Accessible.StaticText
            // Prevent html tags from being read by screen readers
            Accessible.name: text.replace(/<[^>]*>/g, "")
        }

        ButtonGroup {
            id: subscriptionOptions
        }

        Loader {
            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.fillWidth: true
            Layout.maximumWidth: MZTheme.theme.maxHorizontalContentWidth
            Layout.alignment: Qt.AlignHCenter

            active: vpnFlickable.isMobile
            sourceComponent: ColumnLayout {
                spacing: 16

                Repeater {
                    id: productList
                    model: VPNProducts
                    delegate: MZSubscriptionOption {
                        ButtonGroup.group: subscriptionOptions
                    }
                }
            }
        }

        MZButton {
            id: subscribeNow
            objectName: "vpnSubscriptionNeededView"

            text: MZI18n.PurchaseSubscribeNow

            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
            Layout.fillWidth: true

            onClicked: isMobile ? VPNPurchase.subscribe(subscriptionOptions.checkedButton.productId) : VPNPurchase.subscribe("web")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.vSpacingSmall
            Layout.alignment: Qt.AlignHCenter

            spacing: 0

            Item {
                Layout.fillWidth: true
            }

            MZGreyLink {
                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth

                labelText: MZI18n.AboutUsTermsOfService
                onClicked: MZUrlOpener.openUrlLabel("termsOfService")
            }

            Rectangle {
                width: 4
                height: 4
                radius: 2
                opacity: .8
                color: MZTheme.theme.greyLink.defaultColor
            }

            MZGreyLink {
                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth

                labelText: MZI18n.AboutUsPrivacyNotice
                onClicked: MZUrlOpener.openUrlLabel("privacyNotice")
            }

            Item {
                Layout.fillWidth: true
            }
        }

        MZLinkButton {
            id: restorePurchase

            Layout.topMargin: 8
            Layout.alignment: Qt.AlignHCenter

            visible: Qt.platform.os === "ios"
            labelText: MZI18n.RestorePurchaseRestorePurchaseButton
            onClicked: VPNPurchase.restore()
        }

        MZSignOut {
            anchors.bottom: undefined
            anchors.bottomMargin: undefined
            anchors.horizontalCenter: undefined

            Layout.topMargin: 8
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: MZTheme.theme.rowHeight
        }

        //Manual padding for views without a navbar - not ideal, but modifying
        //MZFlickable causes a lot of churn, and is a separate issue
        MZVerticalSpacer {
            Layout.preferredHeight: MZTheme.theme.navBarBottomMargin
        }
    }
}
