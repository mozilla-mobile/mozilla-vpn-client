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

    readonly property string telemetryScreenId: "subscription_needed"

    flickContentHeight: headerLink.implicitHeight + col.implicitHeight + col.anchors.topMargin

    Rectangle {
        color: MZTheme.theme.bgColor
        anchors.fill: parent
    }

    MZHeaderLink {
        id: headerLink

        objectName: "vpnSubscriptionNeededGetHelp"

        labelText: MZI18n.GetHelpLinkTitle
        onClicked: {
            Glean.interaction.getHelpSelected.record({
                screen: vpnFlickable.telemetryScreenId,
            });

            MZNavigator.requestScreen(VPN.ScreenGetHelp)
        }
    }

    ColumnLayout {
        id: col
        objectName: "vpnSubscriptionNeededView"

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
                    id: productListRepeater
                    model: VPNProducts
                    delegate: MZSubscriptionOption {
                        ButtonGroup.group: subscriptionOptions
                    }
                }
                Image {
                    id: spinner
                    visible: productListRepeater.count === 0
                    sourceSize.height: 40
                    fillMode: Image.PreserveAspectFit
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.rightMargin: MZTheme.theme.windowMargin
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    source: "qrc:/nebula/resources/spinner.svg"

                    ParallelAnimation {
                        id: startSpinning

                        running: true

                        PropertyAnimation {
                            target: spinner
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: 300
                        }

                        PropertyAnimation {
                            target: spinner
                            property: "scale"
                            from: 0.7
                            to: 1
                            duration: 300
                        }

                        PropertyAnimation {
                            target: spinner
                            property: "rotation"
                            from: 0
                            to: 360
                            duration: 8000
                            loops: Animation.Infinite
                        }

                    }
                }
            }
        }

        MZButton {
            id: subscribeNow
            objectName: "vpnSubscriptionNeededButton"

            text: MZI18n.PurchaseSubscribeNow

            visible: isMobile ? subscriptionOptions.buttons.length != 0 : true

            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
            Layout.fillWidth: true

            onClicked: {
                Glean.interaction.subscribeNowSelected.record({
                    screen: vpnFlickable.telemetryScreenId,
                });

                isMobile
                    ? VPNPurchase.subscribe(subscriptionOptions.checkedButton.productId)
                    : VPNPurchase.subscribe("web");
            }
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
                objectName: "vpnSubscriptionNeededTermsOfService"

                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth

                labelText: MZI18n.AboutUsTermsOfService
                onClicked: {
                    Glean.interaction.termsOfServiceSelected.record({
                        screen: vpnFlickable.telemetryScreenId,
                    });

                    MZUrlOpener.openUrlLabel("termsOfService")
                }
            }

            Rectangle {
                width: 4
                height: 4
                radius: 2
                opacity: .8
                color: MZTheme.theme.greyLink.defaultColor
            }

            MZGreyLink {
                objectName: "vpnSubscriptionNeededPrivacyNotice"

                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth

                labelText: MZI18n.AboutUsPrivacyNotice
                onClicked: {
                    Glean.interaction.privacyNoticeSelected.record({
                        screen: vpnFlickable.telemetryScreenId,
                    });

                    MZUrlOpener.openUrlLabel("privacyNotice")
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        MZLinkButton {
            id: restorePurchase
            objectName: "vpnSubscriptionNeededRestorePurchase"

            Layout.topMargin: 8
            Layout.alignment: Qt.AlignHCenter

            visible: Qt.platform.os === "ios"
            labelText: MZI18n.RestorePurchaseRestorePurchaseButton
            onClicked: {
                Glean.interaction.alreadyASubscriberSelected.record({
                    screen: vpnFlickable.telemetryScreenId,
                });

                VPNPurchase.restore()
            }
        }

        MZSignOut {
            objectName: "vpnSubscriptionNeededSignOut"

            anchors.bottom: undefined
            anchors.bottomMargin: undefined
            anchors.horizontalCenter: undefined

            Layout.topMargin: 8
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: MZTheme.theme.rowHeight

            preLogoutCallback: () => {
                Glean.interaction.signOutSelected.record({
                    screen: vpnFlickable.telemetryScreenId,
                });
            }
        }

        //Manual padding for views without a navbar - not ideal, but modifying
        //MZFlickable causes a lot of churn, and is a separate issue
        MZVerticalSpacer {
            Layout.preferredHeight: MZTheme.theme.navBarBottomMargin
        }

        Component.onCompleted: {
            Glean.impression.subscriptionNeededScreen.record({
                screen: vpnFlickable.telemetryScreenId,
            });
        }
    }
}
