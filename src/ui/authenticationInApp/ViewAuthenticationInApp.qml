/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

import "../themes/themes.js" as Theme

Item {
    id: viewAuthenticationInApp
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: .3
    }

    Item {
        anchors.fill: parent
        anchors.margins: Theme.vSpacing * 1.5

        VPNDropShadow {
            source: bgColor
            anchors.fill: bgColor
            samples: 20
            transparentBorder: true
            radius: 12.5
            color: "#000000"
            z: -1
            opacity:.2
            cached: true
        }

        Rectangle {
            id: bgColor
            anchors.fill: loader
            anchors.margins: -Theme.windowMargin
            color: Theme.bgColor
            radius: Theme.cornerRadius
        }

        VPNIconButton {
            id: close
            accessibleName: "Cancel authentication" //TODO
            z: 10
            onClicked: VPN.cancelAuthentication()
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: -Theme.windowMargin/2
            anchors.leftMargin: -Theme.windowMargin/2
            VPNIcon {
                source: "../resources/close-dark.svg"
                sourceSize.height: Theme.windowMargin
                sourceSize.width: Theme.windowMargin
                anchors.centerIn: parent
            }
        }

        Loader {
            id: loader
            anchors.fill: parent
            asynchronous: true
        }
    }



    state: VPNAuthInApp.state

    states: [
        State {
            name: VPNAuthInApp.StateInitializing
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationInitializing.qml"
            }
            PropertyChanges {
                target: close
                visible: false
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
            name: VPNAuthInApp.StateVerificationSessionByEmailNeeded
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationVerificationSessionByEmailNeeded.qml"
            }
        },

        State {
            name: VPNAuthInApp.StateVerificationSessionByTotpNeeded
            PropertyChanges {
                target: loader
                source: "ViewAuthenticationVerificationSessionByTotpNeeded.qml"
            }
        }
    ]
}
