/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
// import QtGraphicalEffects 1.0
import Mozilla.VPN 1.0

DropShadow {
    id: dropShadow

    horizontalOffset: 1
    verticalOffset: 1
    radius: 5.5
    color: "#0C0C0D"
    opacity: .1
    state: VPNController.state

    states: [
        State {
            name: VPNController.StateConnecting
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: VPNController.StateConfirming
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: VPNController.StateOn
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: VPNController.StateSwitching
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: VPNController.StateDisconnecting
            PropertyChanges {
                target:dropShadow
                opacity: .1
            }
        },
        State {
            name: VPNController.StateOff
            PropertyChanges {
                target: dropShadow
                opacity: .1
            }
        }
    ]

    Behavior on opacity {
        PropertyAnimation {
            duration: 200
        }
    }
}
