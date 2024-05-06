/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1

Item {
    id: viewAuthenticationInApp

    Loader {
        id: loader
        property bool isReauthFlow: false

        // This is only necessary to fix a text layout bug in Qt 6.4,
        // which can sometimes be pulled in on Linux. Remove this once we move
        // to flatpaks.
        width: item ? item.implicitWidth : 0
        asynchronous: true
        anchors.fill: parent
    }

    MZInAppAuthenticationErrorPopup {
        id: authError
        objectName: "authErrorPopup"
    }

    states: [
        State {
            name: "StateInitializing"
            when: MZAuthInApp.state === MZAuthInApp.StateInitializing || MZAuthInApp.state === MZAuthInApp.StateAuthenticated
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationInitializing.qml"
            }
        },

        State {
            name: "StateStart"
            when: MZAuthInApp.state === MZAuthInApp.StateStart || MZAuthInApp.state === MZAuthInApp.StateCheckingAccount
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationStart.qml"
            }
        },

        State {
            name: "StateSignIn"
            when: MZAuthInApp.state === MZAuthInApp.StateSignIn || MZAuthInApp.state === MZAuthInApp.StateSigningIn
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationSignIn.qml"
            }
        },

        State {
            when: MZAuthInApp.state === MZAuthInApp.StateSignUp || MZAuthInApp.state === MZAuthInApp.StateSigningUp
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationSignUp.qml"
            }
        },

        State {
            name: "StateUnblockCodeNeeded"
            when: MZAuthInApp.state === MZAuthInApp.StateUnblockCodeNeeded || MZAuthInApp.state === MZAuthInApp.StateVerifyingUnblockCode
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationUnblockCodeNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByEmailNeeded"
            when: MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByEmailNeeded || MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionEmailCode
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationVerificationSessionByEmailNeeded.qml"
            }
        },

        State {
            name: "StateVerificationSessionByTotpNeeded"
            when: MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByTotpNeeded || MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionTotpCode
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationVerificationSessionByTotpNeeded.qml"
            }
        },

        State {
            name: "StateIsStubAccount"
            when: MZAuthInApp.state === MZAuthInApp.StateIsStubAccount
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationStubAccount.qml"
            }
        },

        State {
            name: "StateIsSsoAccount"
            when: MZAuthInApp.state === MZAuthInApp.StateIsSsoAccount
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationSsoAccount.qml"
            }
        },

        State {
            name: "StateFallbackInBrowser"
            when: MZAuthInApp.state === MZAuthInApp.StateFallbackInBrowser
            PropertyChanges {
                target: loader
                source: "qrc:/Mozilla/VPN/authenticationInApp/ViewAuthenticationFallbackInBrowser.qml"
            }
        }
    ]
}
