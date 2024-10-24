/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import Mozilla.VPN 1.0

import compat 0.1

MZDropShadow {
    id: dropShadow

    horizontalOffset: 1
    verticalOffset: 1
    radius: 5.5
    color: MZTheme.colors.grey60
    opacity: .1
    state: VPNController.state

    states: [
        State {
            name: "on"
            when: (state === VPNController.StateConnecting ||
                   state === VPNController.StateConfirming ||
                   state === VPNController.StateOn ||
                   state === VPNController.StateSilentSwitching ||
                   state === VPNController.StateSwitching)
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: "off"
            when: (state === VPNController.StateDisconnecting ||
                   state === VPNController.StateOff)
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
