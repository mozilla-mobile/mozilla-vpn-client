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
    id: reauthenticationFlow
    property bool _targetViewCondition: false
    property var _onClose: () => {}
    objectName: "reauthenticationFlow"

    Loader {
        id: loader
        property bool isReauthFlow: true

        anchors.fill: parent
        asynchronous: true

        function cancelAuthenticationFlow() {
            VPN.cancelAuthentication();
            _onClose();
        }
    }

    VPNInAppAuthenticationErrorPopup {
        id: authError
    }

    // The following states are not expected to be set during the reauth flow
    // and thus we do not need to cover them:
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
            ) && !_targetViewCondition
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
            name: "StateAuthenticated"
            when: (
                VPNAuthInApp.state === VPNAuthInApp.StateInitializing
                || VPNAuthInApp.state === VPNAuthInApp.StateAuthenticated
            ) && _targetViewCondition
            StateChangeScript {
                name: "closeReAuth"
                script: {
                    _onClose();
                }
            }
        }
    ]
}
