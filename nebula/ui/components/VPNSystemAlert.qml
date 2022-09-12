/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5

import Mozilla.VPN 1.0

VPNAlert {
    property alias wasmSetAlertState: alertStates.state
    id: alertBox
    alertType: alertTypes.error

    Item {
        id: alertStates
        state: VPN.alert
        states: [
            State {
                name: VPN.NoAlert
                PropertyChanges {
                    target: alertBox
                    visible: false
                }
            },
            State{
                name: VPN.AuthCodeSentAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.success
                    alertText: VPNl18n.InAppAuthEmailTokenResentAlert
                    visible: true
                }
            },
            State {
                name: VPN.AuthenticationFailedAlert
                PropertyChanges {
                    target: alertBox
                    //% "Authentication error"
                    alertText: qsTrId("vpn.alert.authenticationError")
                    //% "Try again"
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true

                    onActionPressed: ()=>{
                        VPN.authenticate();
                    }
                }
            },
            State {
                name: VPN.ConnectionFailedAlert
                PropertyChanges {
                    target: alertBox
                    //% "Unable to connect"
                    alertText: qsTrId("vpn.alert.unableToConnect")
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true
                }
            },
            State {
                name: VPN.NoConnectionAlert
                PropertyChanges {
                    target: alertBox
                    //% "No internet connection"
                    alertText: qsTrId("vpn.alert.noInternet")
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true
                }
            },
            State {
                name: VPN.ControllerErrorAlert
                PropertyChanges {
                    target: alertBox
                    //% "Background service error"
                    alertText: qsTrId("vpn.alert.backendServiceError")
                    //% "Restore"
                    //: Restore a service in case of error.
                    alertActionText: qsTrId("vpn.alert.restore")
                    visible: true
                    onActionPressed: ()=>{
                        VPN.backendServiceRestore();
                    }
                }
            },
            State {
                name: VPN.UnrecoverableErrorAlert
                PropertyChanges {
                    target: alertBox
                    alertText: qsTrId("vpn.alert.backendServiceError")
                    visible: true
                    onActionPressed: ()=>{
                        VPN.backendServiceRestore();
                    }
                }
            },
            State {
                name: VPN.RemoteServiceErrorAlert
                PropertyChanges {
                    target: alertBox
                    //% "Remote service error"
                    alertText: qsTrId("vpn.alert.remoteServiceError")
                    visible: true
                    onActionPressed: ()=>{
                        VPN.backendServiceRestore();
                    }
                }
            },
            State {
                name: VPN.SubscriptionFailureAlert
                PropertyChanges {
                    target: alertBox
                    //% "Subscription failed"
                    alertText: qsTrId("vpn.alert.subscriptionFailureError")
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true
                }
            },
            State {
                name: VPN.GeoIpRestrictionAlert
                PropertyChanges {
                    target: alertBox
                    //% "Operation not allowed from current location"
                    alertText: qsTrId("vpn.alert.getIPRestrictionError")
                    visible: true
                }
            },
            State {
                name: VPN.LogoutAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.success
                    //% "Signed out and device disconnected"
                    alertText: qsTrId("vpn.alert.deviceDisconnectedAndLogout")
                    visible: true
                }
            }
        ]
    }
}
