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
                alertText: qsTr("Authentication error.")
                alertLinkText: qsTr("Try again")
                opacity: 1
                visible: true
            }
        },
        State {
            name: VPN.ConnectionFailedAlert
            PropertyChanges {
                target: alertBox
                alertType: "connection-failed"
                alertText: qsTr("Unable to connect.");
                alertLinkText: qsTr("Try again")
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPN.NoConnectionAlert
            PropertyChanges {
                target: alertBox
                alertType: "no-connection"
                alertText: qsTr("No internet connection.")
                alertLinkText: qsTr("Try again")
                opacity: 1
                visible: true
            }
        },
        State {
            name: VPN.BackendServiceErrorAlert
            PropertyChanges {
                target: alertBox
                alertType: "background-service"
                alertText: qsTr("Background service error.");
                alertLinkText: qsTr("Restore")
                opacity: 1
                visible: true
            }
        },
        State {
            name: VPN.LogoutAlert
            PropertyChanges {
                target: alertBox
                alertText: qsTr("Signed out and device removed");
                opacity: 1
                visible: true
            }
        }

        // TODO - Needed alerts:
        // BackgroundServiceRestored
        // Unable to remove device. Try again

    ]
}
