/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

VPNIcon {
    id: latencyIndicator

    states: [
        // Low latency
        State {
            when: VPNServerLatency.status === VPNServerLatency.StrongSignal
            PropertyChanges {
                target: icon
                source: "qrc:/nebula/resources/server-latency-strong.svg"
            }
        },
        // High latency
        State {
            when: VPNServerLatency.status === VPNServerLatency.WeakSignal
            PropertyChanges {
                target: icon
                source: "qrc:/nebula/resources/server-latency-weak.svg"
            }
        },
        // Very high latency or server unavailable
        State {
            when: VPNServerLatency.status === VPNServerLatency.NoSignal
            PropertyChanges {
                target: icon
                source: "qrc:/nebula/resources/server-latency-unavailable.svg"
            }
        }
    ]

    anchors {
        right: parent.right
        rightMargin: VPNTheme.theme.hSpacing
        top: parent.top
        verticalCenter: parent.verticalCenter
    }
    height: VPNTheme.theme.iconSizeSmall
    sourceSize {
        height: VPNTheme.theme.iconSizeSmall
        width: VPNTheme.theme.iconSizeSmall
    }
    width: VPNTheme.theme.iconSizeSmall
}
