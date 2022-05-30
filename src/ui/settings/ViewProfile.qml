/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1

Item {
    property string _menuTitle: VPNl18n.SubscriptionManagementMenuTitle
    id: subscriptionManagementStates

    Loader {
        property bool _targetViewCondition: (
            VPNProfileFlow.state === VPNProfileFlow.StateInitial
            || VPNProfileFlow.state === VPNProfileFlow.StateReady
        )
        property string _targetViewSource: "qrc:/ui/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml"

        source: "qrc:/ui/authenticationInApp/ViewReauthenticationFlow.qml"

        anchors.fill: parent
    }

    Connections {
        target: VPNProfileFlow

        // TODO: Remove example data
        function onShowProfile() {
            const exampleData = '{
                "created_at": 1626704467,
                "expires_on": 1652970067,
                "is_cancelled": false,
                "payment": {
                    "credit_card_brand": "visa",
                    "credit_card_exp_month": 12,
                    "credit_card_exp_year": 2022,
                    "credit_card_last4": "0016",
                    "provider": "stripe",
                    "type": "credit"
                },
                "plan": {
                    "amount": 499,
                    "currency": "eur",
                    "interval_count": 1,
                    "interval": "month"
                },
                "status": "active",
                "type": "web"
            }';
            VPNSubscriptionData.fromJson(exampleData);
        }
    }

    Component.onCompleted: {
        VPNProfileFlow.start();
    }
}
