/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme


VPNFlickable {
    id: vpnFlickable

    width: window.width
    flickContentHeight: footerContent.height + spacer1.height + vpnPanel.height + featureListBackground.height + (Theme.windowMargin * 4)

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp")
        onClicked: stackview.push(getHelpComponent)
    }

    Component {
        id: getHelpComponent

        VPNGetHelp {
            isSettingsView: false
        }

    }

    Item {
        id: spacer1

        width: parent.width
        height: (Math.max(window.height * .08, Theme.windowMargin * 2))
    }

    VPNPanel {
        id: vpnPanel

        // TODO: Get pricing from settings?
        //: “/mo” stands for “per month”. %1 is replaced by the cost (including currency).
        //% "Subscribe for %1/mo"
        logoTitle: qsTrId("vpn.subscription.title").arg(VPNLocalizer.localizeSubscriptionCurrencyValue())
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
            //% "No activity logs"
            calloutTitle: qsTrId("vpn.subscription.featureTitle1")
            //% "We’re Mozilla. We’re on your side."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle1")
            calloutImage: "../resources/onboarding/onboarding1.svg"
        }

        VPNCallout {
            //% "No one can track you"
            calloutTitle: qsTrId("vpn.subscription.featureTitle2")
            //% "We encrypt your entire device."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle2")
            calloutImage: "../resources/onboarding/onboarding2.svg"
        }

        VPNCallout {
            //: The + after each number stands for “more than”. If you change the number of countries here, please update ViewOnboarding.qml too.
            //% "360+ servers in 30+ countries"
            calloutTitle: qsTrId("vpn.subscription.featureTitle3")
            //% "Protect your access to the web."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle3")
            calloutImage: "../resources/onboarding/onboarding3.svg"
        }

        VPNCallout {
            //% "Connect up to %1 devices"
            calloutTitle: qsTrId("vpn.subscription.featureTitle4").arg(VPNUser.maxDevices)
            //% "We won’t restrict your bandwidth."
            calloutSubtitle: qsTrId("vpn.subscription.featureSubtitle4")
            calloutImage: "../resources/onboarding/onboarding4.svg"
        }

    }

    Item {
        id: spacer2
        anchors.top: featureListBackground.bottom
        height: Math.max(Theme.windowMargin * 2, (window.height - flickContentHeight))
        width: vpnFlickable.width
    }


    ColumnLayout {
        id: footerContent

        anchors.top: spacer2.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(vpnFlickable.width, Theme.maxHorizontalContentWidth)
        spacing: Theme.windowMargin * 1.25

        VPNButton {
            //% "Subscribe now"
            text: qsTrId("vpn.updates.subscribeNow")
            loaderVisible: VPN.subscriptionActive
            onClicked: VPN.subscribe()
        }

        VPNLinkButton {
            //% "Restore purchases"
            labelText: qsTrId("vpn.main.restorePurchases")
            Layout.alignment: Qt.AlignHCenter
            onClicked: VPN.restoreSubscription()
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
