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
    id: viewAuthenticationInApp

    Loader {
        id: loader

        asynchronous: true
        anchors.fill: parent
    }

    VPNInAppAuthenticationErrorPopup {
        id: authError
    }

    states: [
        State {
            name: "StateInitializing"
            when: VPNAuthInApp.state === VPNAuthInApp.StateInitializing
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationInitializing.qml"
            }
        },

        State {
            name: "StateStart"
            when: VPNAuthInApp.state === VPNAuthInApp.StateStart || VPNAuthInApp.state === VPNAuthInApp.StateCheckingAccount
            PropertyChanges {
                target: loader
                // source: "ViewAuthenticationStart.qml"
                source: "ViewDeleteAccount.qml"
            }
        },

        State {
            name: "StateSignIn"
            when: VPNAuthInApp.state === VPNAuthInApp.StateSignIn || VPNAuthInApp.state === VPNAuthInApp.StateSigningIn
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationSignIn.qml"
            }
        },

        State {
            when: VPNAuthInApp.state === VPNAuthInApp.StateSignUp || VPNAuthInApp.state === VPNAuthInApp.StateSigningUp
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationSignUp.qml"
            }
        },

        State {
            name: "StateUnblockCodeNeeded"
            when: VPNAuthInApp.state === VPNAuthInApp.StateUnblockCodeNeeded || VPNAuthInApp.state === VPNAuthInApp.StateVerifyingUnblockCode
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationUnblockCodeNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByEmailNeeded"
            when: VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByEmailNeeded || VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionEmailCode
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationVerificationSessionByEmailNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByTotpNeeded"
            when: VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByTotpNeeded || VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionTotpCode
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationVerificationSessionByTotpNeeded.qml"
            }
        },

        State {
            name: "StateFallbackInBrowser"
            when: VPNAuthInApp.state === VPNAuthInApp.StateFallbackInBrowser
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationFallbackInBrowser.qml"
            }
        }
    ]
}
