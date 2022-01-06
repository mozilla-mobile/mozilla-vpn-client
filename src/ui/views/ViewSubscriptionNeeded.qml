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

    property var wasmView: false

    flickContentHeight: col.y + col.height
    anchors.fill: parent

    Rectangle {
        color: VPNTheme.theme.bgColor
        anchors.fill: parent
    }

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp2")
        onClicked: stackview.push("qrc:/ui/views/ViewGetHelp.qml", {isSettingsView: false})
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
                //% "No activity logs"
                calloutCopy: qsTrId("vpn.subscription.featureTitle1")
                calloutImage: "qrc:/ui/resources/onboarding/onboarding4.svg"
            }

            VPNCallout {
                // "Device level encryption" - String defined in ViewOnboarding.qml
                calloutCopy: qsTrId("vpn.onboarding.headline.1")
                calloutImage: "qrc:/ui/resources/onboarding/onboarding1.svg"
            }

            VPNCallout {
                // Servers in 30+ countries - String defined in ViewOnboarding.qml
                calloutCopy: qsTrId("vpn.onboarding.headline.2")
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
                model: VPNIAP
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
                onClicked: VPNIAP.subscribe(subscriptionOptions.checkedButton.productId)
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

                    // Terms of Service - string defined in VPNAboutUs.qml
                    labelText: qsTrId("vpn.aboutUs.tos2")
                    Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
                    textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter
                    onClicked: VPN.openLink(VPN.LinkTermsOfService)
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

                    // Privacy Notice - string defined in VPNAboutUs.qml
                    labelText: qsTrId("vpn.aboutUs.privacyNotice2")
                    onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                    textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter
                    Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
                }
            }

            VPNSignOut {
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                anchors.horizontalCenter: undefined
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: VPNTheme.theme.rowHeight
            }

            VPNVerticalSpacer {
                Layout.preferredHeight: 60
                Layout.fillWidth: true
            }
        }
    }
}
