/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1

Item {
    id: reauthenticationFlow
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

    MZInAppAuthenticationErrorPopup {
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
                MZAuthInApp.state === MZAuthInApp.StateInitializing
                || MZAuthInApp.state === MZAuthInApp.StateCheckingAccount
            )
            PropertyChanges {
                target: loader
                source: "qrc:/qt/qml/Mozilla/VPN/authenticationInApp/ViewAuthenticationInitializing.qml"
            }
        },

        State {
            name: "StateSignIn"
            when: (
                MZAuthInApp.state === MZAuthInApp.StateSignIn
                || MZAuthInApp.state === MZAuthInApp.StateSigningIn
            )
            PropertyChanges {
                target: loader
                source: "qrc:/qt/qml/Mozilla/VPN/authenticationInApp/ViewAuthenticationSignIn.qml"
            }
        },

        State {
            name: "StateUnblockCodeNeeded"
            when: (
                MZAuthInApp.state === MZAuthInApp.StateUnblockCodeNeeded
                || MZAuthInApp.state === MZAuthInApp.StateVerifyingUnblockCode
            )
            PropertyChanges {
                target: loader
                source: "qrc:/qt/qml/Mozilla/VPN/authenticationInApp/ViewAuthenticationUnblockCodeNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByEmailNeeded"
            when: (
                MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByEmailNeeded
                || MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionEmailCode
            )
            PropertyChanges {
                target: loader
                source: "qrc:/qt/qml/Mozilla/VPN/authenticationInApp/ViewAuthenticationVerificationSessionByEmailNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByTotpNeeded"
            when: (
                MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByTotpNeeded
                || MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionTotpCode
            )
            PropertyChanges {
                target: loader
                source: "qrc:/qt/qml/Mozilla/VPN/authenticationInApp/ViewAuthenticationVerificationSessionByTotpNeeded.qml"
            }
        },

        State {
            name: "StateAuthenticated"
            when: MZAuthInApp.state === MZAuthInApp.StateAuthenticated
            StateChangeScript {
                name: "closeReAuth"
                script: {
                    _onClose();
                }
            }
        }
    ]
}
