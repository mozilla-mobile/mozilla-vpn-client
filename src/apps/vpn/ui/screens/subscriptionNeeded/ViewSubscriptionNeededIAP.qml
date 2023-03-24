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

    flickContentHeight: col.y + col.height

    Rectangle {
        color: MZTheme.theme.bgColor
        anchors.fill: parent
    }

    MZHeaderLink {
        id: headerLink

        labelText: MZI18n.GetHelpLinkTitle
        onClicked: MZNavigator.requestScreen(VPN.ScreenGetHelp)
    }

    Image {
        id: logo
        sourceSize.height: 48
        sourceSize.width: 48
        smooth: true
        source: "qrc:/ui/resources/logo.svg"
        anchors.horizontalCenter: parent.horizontalCenter
        antialiasing: true
        anchors.top: headerLink.bottom
        anchors.topMargin: 20
    }

    MZHeadline {
        id: headline
        text: MZI18n.ProductName
        anchors.top: logo.bottom
        anchors.topMargin: MZTheme.theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
    }

    ColumnLayout {
        id: col
        anchors.top: headline.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 24

        ColumnLayout {
            id: featureList
            spacing: MZTheme.theme.windowMargin
            Layout.maximumWidth: MZTheme.theme.maxHorizontalContentWidth
            Layout.topMargin: 24
            Layout.alignment: Qt.AlignHCenter

            MZCallout {
                calloutCopy: MZI18n.SubscriptionManagementValueProp1
                calloutImage: "qrc:/ui/resources/onboarding/onboarding4.svg"
            }

            MZCallout {
                calloutCopy: MZI18n.SubscriptionManagementValueProp2
                calloutImage: "qrc:/ui/resources/onboarding/onboarding1.svg"
            }

            MZCallout {
                calloutCopy: MZI18n.SubscriptionManagementValueProp3
                calloutImage: "qrc:/ui/resources/onboarding/onboarding2.svg"
            }

            MZCallout {
                //% "Connect up to %1 devices"
                //: %1 is the number of devices.
                //: Note: there is currently no support for proper plurals
                calloutCopy: qsTrId("vpn.subscription.featureTitle4").arg(
                                VPNUser.maxDevices)
                calloutImage: "qrc:/ui/resources/onboarding/onboarding3.svg"
            }
        }

        ColumnLayout {
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.maximumWidth: MZTheme.theme.maxHorizontalContentWidth
            Layout.alignment: Qt.AlignHCenter
            spacing: MZTheme.theme.windowMargin

            ButtonGroup {
                id: subscriptionOptions
            }

            Repeater {
                id: productList
                model: VPNProducts
                delegate: MZSubscriptionOption {}
            }
        }

        ColumnLayout {
            id: footerInfo
            spacing: MZTheme.theme.windowMargin

            MZButton {
                id: subscribeNow
                objectName: "vpnSubscriptionNeededView"

                text: MZI18n.PurchaseSubscribeNow

                Layout.topMargin: MZTheme.theme.windowMargin
                Layout.leftMargin: MZTheme.theme.windowMargin
                Layout.rightMargin: MZTheme.theme.windowMargin
                Layout.fillWidth: true
                Layout.maximumWidth: MZTheme.theme.maxHorizontalContentWidth
                onClicked: VPNPurchase.subscribe(subscriptionOptions.checkedButton.productId)
            }
            GridLayout {
                id: grid
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                columnSpacing: 0
                columns: 3
                Component.onCompleted: {
                    if (implicitWidth > window.width) {
                        flow = Grid.TopToBottom
                    }
                }

                MZGreyLink {
                    id: termsOfService

                    labelText: MZI18n.AboutUsTermsOfService
                    Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
                    textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter
                    onClicked: MZUrlOpener.openUrlLabel("termsOfService")
                }

                Rectangle {
                    width: 4
                    height: 4
                    radius: 2
                    Layout.alignment: Qt.AlignHCenter
                    color: MZTheme.theme.greyLink.defaultColor
                    visible: parent.flow != Grid.TopToBottom
                    opacity: .8
                }

                MZGreyLink {
                    id: privacyNotice

                    labelText: MZI18n.AboutUsPrivacyNotice
                    onClicked: MZUrlOpener.openUrlLabel("privacyNotice")
                    textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter
                    Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
                }
            }

            MZLinkButton {
                id: restorePurchase
                visible: Qt.platform.os === "ios"

                // Already a subscriber?
                labelText: MZI18n.RestorePurchaseRestorePurchaseButton
                Layout.alignment: Qt.AlignHCenter
                onClicked: VPNPurchase.restore()
            }

            MZSignOut {
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                anchors.horizontalCenter: undefined
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: MZTheme.theme.rowHeight
            }

            MZVerticalSpacer {
                Layout.preferredHeight: 10
                Layout.fillWidth: true
            }
        }
    }
}
