/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme


VPNFlickable {
    id: vpnFlickable

    property var wasmView: false

    Component.onCompleted: {
        if (wasmView) {
            vpnPanel.logoTitle = qsTrId("vpn.subscription.title").arg("5.99")
        }
    }

    width: window.width
    flickContentHeight: footerContent.height + spacer1.height + vpnPanel.height + featureListBackground.height + (Theme.windowMargin * 4)

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp")
        onClicked: stackview.push("../views/ViewGetHelp.qml")
    }

    Item {
        id: spacer1

        width: parent.width
        height: (Math.max(window.safeContentHeight * .04, Theme.windowMargin * 2))
    }

    VPNPanel {
        id: vpnPanel

        //: “/mo” stands for “per month”. %1 is replaced by the cost (including currency).
        //% "Subscribe for %1/mo"
        logoTitle: qsTrId("vpn.subscription.title").arg(VPNIAP.priceValue)
        //% "30-day money-back guarantee"
        logoSubtitle: qsTrId("vpn.subscription.moneyBackGuarantee")
        anchors.top: spacer1.bottom
        logo: "../resources/logo.svg"
        logoSize: 48
    }

    VPNDropShadow {
        anchors.fill: featureListBackground
        source: featureListBackground
        z: -2
    }

    Rectangle {
        id: featureListBackground

        anchors.fill: featureList
        anchors.topMargin: -(Theme.windowMargin * 2)
        anchors.bottomMargin: -(Theme.windowMargin * 2)
        color: Theme.white
        radius: 4
        z: -1
    }

    ColumnLayout {
        id: featureList

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: vpnPanel.bottom
        anchors.topMargin: Theme.windowMargin * 4
        width: Math.min(vpnFlickable.width - Theme.windowMargin * 2, Theme.maxHorizontalContentWidth)
        spacing: Theme.vSpacing

        VPNCallout {
            // "Device level encryption" - String defined in ViewOnboarding.qml
            calloutTitle: qsTrId("vpn.onboarding.headline.1")
            //% "We encrypt your entire device."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle2")
            calloutImage: "../resources/onboarding/onboarding1.svg"
        }

        VPNCallout {
            // Servers in 30+ countries - String defined in ViewOnboarding.qml
            calloutTitle: qsTrId("vpn.onboarding.headline.2")
            //% "Protect your access to the web."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle3")
            calloutImage: "../resources/onboarding/onboarding2.svg"
        }

        VPNCallout {
            //% "Connect up to %1 devices"
            //: %1 is the number of devices.
            //: Note: there is currently no support for proper plurals
            calloutTitle: qsTrId("vpn.subscription.featureTitle4").arg(VPNUser.maxDevices)
            //% "We won’t restrict your bandwidth."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle4")
            calloutImage: "../resources/onboarding/onboarding3.svg"
        }

        VPNCallout {
            //% "No activity logs"
            calloutTitle: qsTrId("vpn.subscription.featureTitle1")
            //% "We’re Mozilla. We’re on your side."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle1")
            calloutImage: "../resources/onboarding/onboarding4.svg"
        }

    }

    Item {
        id: spacer2
        anchors.top: featureListBackground.bottom
        height: Math.max(Theme.windowMargin * 2, (window.safeContentHeight - flickContentHeight))
        width: vpnFlickable.width
    }


    ColumnLayout {
        id: footerContent

        anchors.top: spacer2.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(vpnFlickable.width, Theme.maxHorizontalContentWidth)
        spacing: 0

        VPNButton {
            id: subscribeNow

            //% "Subscribe now"
            text: qsTrId("vpn.updates.subscribeNow")
            Layout.alignment: Qt.AlignHCenter
            loaderVisible: false
            onClicked: VPNIAP.subscribe()
        }

        Rectangle { // vertical spacer
            color: "transparent"
            Layout.preferredHeight: Theme.windowMargin
            Layout.fillWidth: true
        }

        GridLayout {
            id: grid
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            columnSpacing: 0
            Component.onCompleted: columns = (termsOfService.width > subscribeNow.width / 2 || privacyNotice.width > subscribeNow.width / 2) ? 1 : 3;

            VPNGreyLink {
                id: termsOfService

                // Terms of Service - string definted in VPNAboutUs.qml
                labelText: qsTrId("vpn.aboutUs.tos")
                Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
                textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter
                onClicked: VPN.openLink(VPN.LinkTermsOfService)
            }

            Rectangle {
                id: centerDivider

                width: 4
                height: 4
                radius: 2
                Layout.alignment: Qt.AlignHCenter
                color: Theme.greyLink.defaultColor
                visible: (grid.columns > 1)
                opacity: .8
            }

            VPNGreyLink {
                id: privacyNotice

                // Privacy Notice - string defined in VPNAboutUs.qml
                labelText: qsTrId("vpn.aboutUs.privacyNotice")
                onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter
                Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
            }
        }

        Rectangle { // vertical spacer
            color: "transparent"
            Layout.preferredHeight: Theme.windowMargin * 1.5
            Layout.fillWidth: true
        }

        Rectangle { // vertical spacer
            color: "transparent"
            Layout.preferredHeight: Theme.windowMargin * .5
            Layout.fillWidth: true
        }

        VPNSignOut {
            anchors.bottom: undefined
            anchors.bottomMargin: undefined
            anchors.horizontalCenter: undefined
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                VPNController.logout();
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.windowMargin * 2
            color: "transparent"
        }


    }

}
