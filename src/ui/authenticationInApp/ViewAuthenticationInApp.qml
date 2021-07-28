/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

Item {
    id: viewAuthenticationInApp

    Loader {
        id: loader

        asynchronous: true
        anchors.fill: parent
    }

    state: VPNAuthInApp.state

    states: [
        State {
            name: VPNAuthInApp.StateInitializing
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationInitializing.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateStart
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationStart.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateAccountStatus
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationAccountStatus.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateSignIn
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationSignIn.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateSignUp
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationSignUp.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateEmailVerification
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationEmailVerification.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateAccountVerification
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationAccountVerification.qml"
            }
        }
    ]
}
