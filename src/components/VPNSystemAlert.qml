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
                //% "Authentication error."
                alertText: qsTrId("authenticationError")
                //% "Try again."
                alertLinkText: qsTrId("tryAgain")
                opacity: 1
                visible: true
            }
        },
        State {
            name: VPN.ConnectionFailedAlert
            PropertyChanges {
                target: alertBox
                alertType: "connection-failed"
                //% "Unable to connect."
                alertText: qsTrId("unableToConnect");
                alertLinkText: qsTrId("tryAgain")
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
                alertText: qsTrId("noInternet")
                alertLinkText: qsTrId("tryAgain")
                opacity: 1
                visible: true
            }
        },
        State {
            name: VPN.BackendServiceErrorAlert
            PropertyChanges {
                target: alertBox
                alertType: "background-service"
                //% "Background service error."
                alertText: qsTrId("backgroundServiceError");
                //% "Restore"
                //: Restore a service in case of an Error
                alertLinkText: qsTrId("restore")
                opacity: 1
                visible: true
            }
        },
        State {
            name: VPN.LogoutAlert
            PropertyChanges {
                target: alertBox
                //% "Signed out and device removed"
                alertText: qsTrId("deviceRemovedAndLogout");
                opacity: 1
                visible: true
            }
        }

        // TODO - Needed alerts:
        // BackgroundServiceRestored
        // Unable to remove device. Try again

    ]
}
