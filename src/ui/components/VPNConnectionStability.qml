/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RowLayout {
    id: stability

    spacing: 0
    opacity: 0
    state: VPNConnectionHealth.stability
    states: [
        State {
            name: VPNConnectionHealth.Stable

            PropertyChanges {
                target: stability
                visible: false
                opacity: 0
            }

        },
        State {
            name: VPNConnectionHealth.Unstable

            PropertyChanges {
                target: stability
                opacity: 1
                visible: true
            }

        },
        State {
            name: VPNConnectionHealth.NoSignal

            PropertyChanges {
                target: stability
                opacity: 1
                visible: true
            }

        }
    ]
    transitions: [
        Transition {
            from: VPNConnectionHealth.Stable
            to: VPNConnectionHealth.Unstable || VPNConnectionHealth.NoSignal

            SequentialAnimation {
                PropertyAction {
                    target: stability
                    property: "visible"
                    value: true
                }

                VPNStabilityLabelActions {
                }

                NumberAnimation {
                    target: stability
                    property: "opacity"
                    to: 1
                    duration: 200
                }

            }

        },
        Transition {
            SequentialAnimation {
                NumberAnimation {
                    target: iconLabelWrapper
                    property: "opacity"
                    to: 0
                    duration: 100
                }

                VPNStabilityLabelActions {
                }

                NumberAnimation {
                    target: iconLabelWrapper
                    property: "opacity"
                    to: 1
                    duration: 200
                }

            }

        }
    ]

    RowLayout {
        id: iconLabelWrapper

        spacing: 0
        Layout.minimumWidth: 60

        Rectangle {
            height: 22
            width: 15
            color: "transparent"
            Layout.rightMargin: 8

            Image {
                id: warningIcon

                sourceSize.height: 15
                sourceSize.width: 15
                fillMode: Image.PreserveAspectFit
            }

        }

        VPNInterLabel {
            id: stabilityLabel
        }

    }

    Rectangle {
        Layout.preferredHeight: 5
        Layout.preferredWidth: 5
        color: "#FFFFFF"
        opacity: 0.8
        radius: 3
        Layout.leftMargin: 12
        Layout.rightMargin: 12
        Layout.bottomMargin: 4
    }

    VPNInterLabel {
        //% "Check Connection"
        text: qsTrId("vpn.connectionStability.checkConnection")
        color: "#FFFFFF"
        opacity: 0.8
    }

}
