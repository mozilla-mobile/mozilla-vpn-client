/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import Mozilla.VPN 1.0

import compat 0.1

VPNDropShadow {
    id: dropShadow

    horizontalOffset: 1
    verticalOffset: 1
    radius: 5.5
    color: "#0C0C0D"
    opacity: .1
    state: MZModules["vpn"].controller.state

    states: [
        State {
            name: MZModules["vpn"].controller.StateConnecting
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: MZModules["vpn"].controller.StateConfirming
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: MZModules["vpn"].controller.StateOn
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: MZModules["vpn"].controller.StateSwitching
            PropertyChanges {
                target: dropShadow
                opacity: .3
            }
        },
        State {
            name: MZModules["vpn"].controller.StateDisconnecting
            PropertyChanges {
                target:dropShadow
                opacity: .1
            }
        },
        State {
            name: MZModules["vpn"].controller.StateOff
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
