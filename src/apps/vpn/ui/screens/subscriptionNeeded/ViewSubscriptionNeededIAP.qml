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

    flickContentHeight: col.y + col.height

    Rectangle {
        color: VPNTheme.theme.bgColor
        anchors.fill: parent
    }

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp2")
        onClicked: VPNNavigator.requestScreen(VPNNavigator.ScreenGetHelp)
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

    VPNHeadline {
        id: headline
        text: qsTrId("vpn.main.productName")
        anchors.top: logo.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
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
            spacing: VPNTheme.theme.windowMargin
            Layout.maximumWidth: VPNTheme.theme.maxHorizontalContentWidth
            Layout.topMargin: 24
            Layout.alignment: Qt.AlignHCenter

            VPNCallout {
                calloutCopy: VPNI18n.SubscriptionManagementValueProp1
                calloutImage: "qrc:/ui/resources/onboarding/onboarding4.svg"
            }

            VPNCallout {
                calloutCopy: VPNI18n.SubscriptionManagementValueProp2
                calloutImage: "qrc:/ui/resources/onboarding/onboarding1.svg"
            }

            VPNCallout {
                calloutCopy: VPNI18n.SubscriptionManagementValueProp3
                calloutImage: "qrc:/ui/resources/onboarding/onboarding2.svg"
            }

            VPNCallout {
                //% "Connect up to %1 devices"
                //: %1 is the number of devices.
                //: Note: there is currently no support for proper plurals
                calloutCopy: qsTrId("vpn.subscription.featureTitle4").arg(
                                VPNUser.maxDevices)
                calloutImage: "qrc:/ui/resources/onboarding/onboarding3.svg"
            }
        }

        ColumnLayout {
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.maximumWidth: VPNTheme.theme.maxHorizontalContentWidth
            Layout.alignment: Qt.AlignHCenter
            spacing: VPNTheme.theme.windowMargin

            ButtonGroup {
                id: subscriptionOptions
            }

            Repeater {
                id: productList
                model: VPNProducts
                delegate: VPNSubscriptionOption {}
            }
        }

        ColumnLayout {
            id: footerInfo
            spacing: VPNTheme.theme.windowMargin

            VPNButton {
                id: subscribeNow

                //% "Subscribe now"
                text: qsTrId("vpn.updates.subscribeNow")

                Layout.topMargin: VPNTheme.theme.windowMargin
                Layout.leftMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin
                Layout.fillWidth: true
                Layout.maximumWidth: VPNTheme.theme.maxHorizontalContentWidth
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

                VPNGreyLink {
                    id: termsOfService

                    // Terms of Service - string defined in ViewAboutUs.qml
                    labelText: qsTrId("vpn.aboutUs.tos2")
                    Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
                    textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter
                    onClicked: VPNUrlOpener.openUrlLabel("termsOfService")
                }

                Rectangle {
                    width: 4
                    height: 4
                    radius: 2
                    Layout.alignment: Qt.AlignHCenter
                    color: VPNTheme.theme.greyLink.defaultColor
                    visible: parent.flow != Grid.TopToBottom
                    opacity: .8
                }

                VPNGreyLink {
                    id: privacyNotice

                    // Privacy Notice - string defined in ViewAboutUs.qml
                    labelText: qsTrId("vpn.aboutUs.privacyNotice2")
                    onClicked: VPNUrlOpener.openUrlLabel("privacyNotice")
                    textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter
                    Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
                }
            }

            VPNLinkButton {
                id: restorePurchase
                visible: Qt.platform.os === "ios"

                // Already a subscriber?
                labelText: VPNI18n.RestorePurchaseRestorePurchaseButton
                Layout.alignment: Qt.AlignHCenter
                onClicked: VPNPurchase.restore()
            }

            VPNSignOut {
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                anchors.horizontalCenter: undefined
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: VPNTheme.theme.rowHeight
            }

            VPNVerticalSpacer {
                Layout.preferredHeight: 10
                Layout.fillWidth: true
            }
        }
    }
}
