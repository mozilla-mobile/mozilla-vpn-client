/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

import QtQuick.Layouts 1.14


Item {
    id: viewInitialize

    VPNHeaderLink {
        id: headerLink
        objectName: "getHelpLink"

        labelText: qsTrId("vpn.main.getHelp2")
        onClicked: {
            Sample.getHelpClickedInitialize.record();
            getHelpViewNeeded();
        }
    }

    ColumnLayout {
        anchors {
            left: parent.left
            top: parent.top
        }
        width: parent.width

        Repeater {
            model: VPNSubscriptionModel
            delegate: Text {
                text: key + ": " + values
            }
        }

        VPNButton {
            text: VPNSubscriptionModel.initialized ? ":)" : ":("
            onClicked: {
                const data = '{"created_at":1626704467,"expires_on":1652970067,"is_cancelled":false,"payment":{"credit_card_brand":"visa","credit_card_exp_month":12,"credit_card_exp_year":2022,"credit_card_last4":"0016","provider":"stripe","type":"credit"},"plan":{"amount":499,"currency":"eur","interval_count":1,"interval":"month"},"status":"active","type":"web"}';

                VPNSubscriptionModel.fromJson(data);
            }
        }
    }

    VPNPanel {
        logo: "qrc:/ui/resources/logo.svg"
        logoTitle: qsTrId("vpn.main.productName")
        //% "A fast, secure and easy to use VPN. Built by the makers of Firefox."
        logoSubtitle: qsTrId("vpn.main.productDescription")
        logoSize: 80
        height: parent.height - (getStarted.height + getStarted.anchors.bottomMargin + learnMore.height + learnMore.anchors.bottomMargin)
    }

    VPNButton {
        id: getStarted
        objectName: "getStarted"

        anchors.bottom: learnMore.top
        anchors.bottomMargin: 24
        //% "Get started"
        text: qsTrId("vpn.main.getStarted")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPN.getStarted()

    }

    VPNFooterLink {
        id: learnMore
        objectName: "learnMoreLink"

        //% "Learn more"
        labelText: qsTrId("vpn.main.learnMore")
        onClicked: {
            Sample.onboardingOpened.record();
            stackview.push("ViewOnboarding.qml");
        }

    }

}
