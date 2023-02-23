/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

VPNIcon {
    property int score: VPNServerLatency.NoData

    id: latencyIndicator

    states: [
        // Low latency
        State {
            when: (score === VPNServerLatency.Good ||
                score === VPNServerLatency.Excellent)
            PropertyChanges {
                target: latencyIndicator
                source: "qrc:/nebula/resources/server-latency-strong.svg"
                visible: true
            }
        },
        // Moderate latency
        State {
            when: (score === VPNServerLatency.Moderate)
            PropertyChanges {
                target: latencyIndicator
                source: "qrc:/nebula/resources/server-latency-moderate.svg"
                visible: true
            }
        },
        // High latency
        State {
            when: (score === VPNServerLatency.Poor)
            PropertyChanges {
                target: latencyIndicator
                source: "qrc:/nebula/resources/server-latency-weak.svg"
                visible: true
            }
        },
        // Very high latency or server unavailable
        State {
            when: score === VPNServerLatency.Unavailable
            PropertyChanges {
                target: latencyIndicator
                source: "qrc:/nebula/resources/server-latency-unavailable.svg"
                visible: true
            }
        },
        // No data
        State {
            when: score === VPNServerLatency.NoData
            PropertyChanges {
                target: latencyIndicator
                source: ""
                visible: false
            }
        }
    ]

    height: VPNTheme.theme.iconSize * 1.5
    sourceSize {
        height: VPNTheme.theme.iconSize * 1.5
        width: VPNTheme.theme.iconSize * 1.5
    }
    width: VPNTheme.theme.iconSize * 1.5
}
