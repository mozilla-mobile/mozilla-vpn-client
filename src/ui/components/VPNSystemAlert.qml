/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import Mozilla.VPN 1.0

VPNAlert {
    id: alertBox

    visible: false
    state: VPN.alert
    states: [
        State {
            name: VPN.NoAlert

            PropertyChanges {
                target: alertBox
                opacity: 0
                visible: false
            }

        },
        State {
            name: VPN.updateRecommended

            PropertyChanges {
                target: alertBox
                opacity: 0
                visible: false
            }

        },
        State {
            name: VPN.AuthenticationFailedAlert

            PropertyChanges {
                target: alertBox
                alertType: "authentication-failed"
                //% "Authentication error"
                alertText: qsTrId("vpn.alert.authenticationError")
                //% "Try again"
                alertLinkText: qsTrId("vpn.alert.tryAgain")
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.ConnectionFailedAlert

            PropertyChanges {
                target: alertBox
                alertType: "connection-failed"
                //% "Unable to connect"
                alertText: qsTrId("vpn.alert.unableToConnect")
                alertLinkText: qsTrId("vpn.alert.tryAgain")
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.NoConnectionAlert

            PropertyChanges {
                target: alertBox
                alertType: "no-connection"
                //% "No internet connection"
                alertText: qsTrId("vpn.alert.noInternet")
                alertLinkText: qsTrId("vpn.alert.tryAgain")
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.ControllerErrorAlert

            PropertyChanges {
                target: alertBox
                alertType: "backend-service"
                //% "Background service error"
                alertText: qsTrId("vpn.alert.backendServiceError")
                //% "Restore"
                //: Restore a service in case of error.
                alertLinkText: qsTrId("vpn.alert.restore")
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.UnrecoverableErrorAlert

            PropertyChanges {
                target: alertBox
                alertType: "backend-service"
                alertText: qsTrId("vpn.alert.backendServiceError")
                alertLinkText: ""
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.RemoteServiceErrorAlert

            PropertyChanges {
                target: alertBox
                alertType: "backend-service"
                //% "Remote service error"
                alertText: qsTrId("vpn.alert.remoteServiceError")
                alertLinkText: ""
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.SubscriptionFailureAlert

            PropertyChanges {
                target: alertBox
                alertType: "subscription-failed"
                //% "Subscription failed"
                alertText: qsTrId("vpn.alert.subscriptionFailureError")
                alertLinkText: qsTrId("vpn.alert.tryAgain")
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.GeoIpRestrictionAlert

            PropertyChanges {
                target: alertBox
                alertType: "geoip-restriction"
                //% "Operation not allowed from current location"
                alertText: qsTrId("vpn.alert.getIPRestrictionError")
                alertLinkText: ""
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.LogoutAlert

            PropertyChanges {
                target: alertBox
                //% "Signed out and device removed"
                alertText: qsTrId("vpn.alert.deviceRemovedAndLogout")
                opacity: 1
                visible: true
            }

        }
    ]
}
