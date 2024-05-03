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

    id: viewDeleteAccount

    Loader {
        id: loader
        property bool isReauthFlow: true

        anchors.fill: parent
        asynchronous: true

        function cancelAuthenticationFlow() {
            VPN.cancelReauthentication();
            MZNavigator.requestPreviousScreen();
        }
    }

    MZInAppAuthenticationErrorPopup {
        id: authError
    }

    // The following states are not expected to be set during the
    // account deletion flow and thus we do not need to cover them:
    // - StateStart
    // - StateSignUp
    // - StateSigningUp
    // - StateFallbackInBrowser
    states: [
        State {
            name: "StateInitializing"
            when: (
                MZAuthInApp.state === MZAuthInApp.StateInitializing
                || MZAuthInApp.state === MZAuthInApp.StateAuthenticated
                || MZAuthInApp.state === MZAuthInApp.StateCheckingAccount
            )
            PropertyChanges {
                target: loader
                source: "../authenticationInApp/ViewAuthenticationInitializing.qml"
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
                source: "../authenticationInApp/ViewAuthenticationSignIn.qml"
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
                source: "../authenticationInApp/ViewAuthenticationUnblockCodeNeeded.qml"
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
                source: "../authenticationInApp/ViewAuthenticationVerificationSessionByEmailNeeded.qml"
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
                source: "../authenticationInApp/ViewAuthenticationVerificationSessionByTotpNeeded.qml"
            }
        },

        State {
            name: "StateAccountDeletionRequest"
            when: (
                MZAuthInApp.state === MZAuthInApp.StateAccountDeletionRequest
                || MZAuthInApp.state === MZAuthInApp.StateDeletingAccount
            )
            PropertyChanges {
                target: loader
                source: "ViewDeleteAccountRequest.qml"
            }
        }
    ]

    Component.onCompleted: {
        VPN.requestDeleteAccount();
    }

    // Catch edge-cases where the user navigates away from the
    // delete account view (by clicking something in the systray, or
    // by clicking Help -> Some other nav bar option).
    Component.onDestruction: VPN.cancelReauthentication();
}
