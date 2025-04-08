/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0

MZAlert {
    property alias wasmSetAlertState: alertStates.state
    id: alertBox
    alertType: alertTypes.error

    Item {
        id: alertStates
        state: MZErrorHandler.alert
        states: [
            State {
                name: MZErrorHandler.NoAlert
                PropertyChanges {
                    target: alertBox
                    visible: false
                }
            },
            State{
                name: MZErrorHandler.AuthCodeSentAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.success
                    alertText: MZI18n.InAppAuthEmailTokenResentAlert
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.AuthenticationFailedAlert
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
                name: MZErrorHandler.ConnectionFailedAlert
                PropertyChanges {
                    target: alertBox
                    //% "Unable to connect"
                    alertText: qsTrId("vpn.alert.unableToConnect")
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.NoConnectionAlert
                PropertyChanges {
                    target: alertBox
                    alertText: MZI18n.GlobalNoInternetConnection
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.ControllerErrorAlert
                PropertyChanges {
                    target: alertBox
                    //% "Background service error"
                    alertText: qsTrId("vpn.alert.backendServiceError")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.SplitTunnelErrorAlert
                PropertyChanges {
                    target: alertBox
                    alertText: MZI18n.SplittunnelErrorActivating
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.UnrecoverableErrorAlert
                PropertyChanges {
                    target: alertBox
                    alertText: qsTrId("vpn.alert.backendServiceError")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.RemoteServiceErrorAlert
                PropertyChanges {
                    target: alertBox
                    //% "Remote service error"
                    alertText: qsTrId("vpn.alert.remoteServiceError")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.SubscriptionFailureAlert
                PropertyChanges {
                    target: alertBox
                    //% "Subscription failed"
                    alertText: qsTrId("vpn.alert.subscriptionFailureError")
                    alertActionText: qsTrId("vpn.alert.tryAgain")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.GeoIpRestrictionAlert
                PropertyChanges {
                    target: alertBox
                    //% "Operation not allowed from current location"
                    alertText: qsTrId("vpn.alert.getIPRestrictionError")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.LogoutAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.success
                    //% "Signed out and device disconnected"
                    alertText: qsTrId("vpn.alert.deviceDisconnectedAndLogout")
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.CopiedToClipboardConfirmationAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.success
                    alertText: MZI18n.GlobalCopied
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.SampleWarningAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.warning
                    alertText: "Sample warning alert"
                    visible: true
                }
            },
            State {
                name: MZErrorHandler.SampleInfoAlert
                PropertyChanges {
                    target: alertBox
                    alertType: alertTypes.info
                    alertText: "Sample info alert"
                    visible: true
                }
            }
        ]
    }
}
