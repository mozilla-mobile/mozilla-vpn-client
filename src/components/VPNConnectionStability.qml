import QtQuick 2.0

import Mozilla.VPN 1.0

Text {
    id: stability
    state: VPNConnectionHealth.stability

    states: [
        State {
            name: VPNConnectionHealth.Stable
            PropertyChanges {
                target: stability
                text: qsTr("")
            }
        },
        State {
            name: VPNConnectionHealth.Unstable
            PropertyChanges {
                target: stability
                text: qsTr("Unstable")
            }
        },
        State {
            name: VPNConnectionHealth.NoSignal
            PropertyChanges {
                target: stability
                text: qsTr("NoSignal")
            }
        }
    ]

    font.pixelSize: 15
    height: 22
}
