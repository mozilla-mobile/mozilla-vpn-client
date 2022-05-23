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
    property bool _menuVisible: false
    id: subscriptionManagementStates

    Loader {
        id: loader
        property bool isReauthFlow: true

        anchors.fill: parent
        asynchronous: true

        function cancelAuthenticationFlow() {
            VPN.cancelAuthentication();

            settingsStackView.pop();
        }
    }

    VPNInAppAuthenticationErrorPopup {
        id: authError
    }

    // The following states are not expected to be set during the Subscription
    // Management auth flow and thus we do not need to cover them:
    // - StateStart
    // - StateSignUp
    // - StateSigningUp
    // - StateFallbackInBrowser
    states: [
        State {
            name: "StateInitializing"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateInitializing
                || VPNAuthInApp.state === VPNAuthInApp.StateAuthenticated
                || VPNAuthInApp.state === VPNAuthInApp.StateCheckingAccount
            ) && !VPNSubscriptionData.initialized
            PropertyChanges {
                target: loader
                source: "qrc:/ui/authenticationInApp/ViewAuthenticationInitializing.qml"
            }
        },

        State {
            name: "StateSignIn"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateSignIn
                || VPNAuthInApp.state === VPNAuthInApp.StateSigningIn
            )
            PropertyChanges {
                target: loader
                source: "qrc:/ui/authenticationInApp/ViewAuthenticationSignIn.qml"
            }
        },

        State {
            name: "StateUnblockCodeNeeded"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateUnblockCodeNeeded
                || VPNAuthInApp.state === VPNAuthInApp.StateVerifyingUnblockCode
            )
            PropertyChanges {
                target: loader
                source: "qrc:/ui/authenticationInApp/ViewAuthenticationUnblockCodeNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByEmailNeeded"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByEmailNeeded
                || VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionEmailCode
            )
            PropertyChanges {
                target: loader
                source: "qrc:/ui/authenticationInApp/ViewAuthenticationVerificationSessionByEmailNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByTotpNeeded"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByTotpNeeded
                || VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionTotpCode
            )
            PropertyChanges {
                target: loader
                source: "qrc:/ui/authenticationInApp/ViewAuthenticationVerificationSessionByTotpNeeded.qml"
            }
        },

        State {
            name: "StateSubscriptionManagement"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateInitializing
                || VPNAuthInApp.state === VPNAuthInApp.StateAuthenticated
            ) && VPNSubscriptionData.initialized
            PropertyChanges {
                target: loader
                source: "qrc:/ui/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml"
            }
        }
    ]

    Connections {
        target: VPN

        // TODO: Remove example data
        function onSubscriptionManagementNeeded() {
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
        VPN.getSubscriptionDetails();
    }
}
