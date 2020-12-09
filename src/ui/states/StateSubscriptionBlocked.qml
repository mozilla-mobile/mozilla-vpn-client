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

    Item {
        id: spacer1

        width: parent.width
        height: (Math.max(window.safeContentHeight * .04, Theme.windowMargin * 2))
    }

    VPNPanel {
        id: vpnPanel

        //% "Error confirming subscription…"
        logoTitle: qsTrId("vpn.subscriptionBlocked.title").arg(VPNIAP.priceValue)
        //% "Another Firefox Account has already subscribed using this Apple ID\nVisit our help center below to learn more about how to manage your subscriptions"
        logoSubtitle: qsTrId("vpn.subscriptionBlocked.subtitle")
        anchors.top: spacer1.bottom
        logo: "../resources/logo.svg"
        logoSize: 48
    }

    ColumnLayout {
        id: footerContent

        anchors.top: vpnPanel.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(vpnFlickable.width, Theme.maxHorizontalContentWidth)
        spacing: 0

        VPNButton {
            id: getHelp

            //% "Get Help"
            text: qsTrId("vpn.subscriptionBlocked.getHelp")
            Layout.alignment: Qt.AlignHCenter
            loaderVisible: false
            onClicked: VPN.openLink(VPN.LinkSubscriptionBlocked)
        }

        Rectangle { // vertical spacer
            color: "transparent"
            Layout.preferredHeight: Theme.windowMargin * 1.5
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
