/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RowLayout {
    id: stability
    Layout.preferredHeight: Theme.controllerInterLineHeight

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
            height: 16
            width: 14
            color: "transparent"
            Layout.rightMargin: 6

            Image {
                id: warningIcon

                sourceSize.height: 14
                sourceSize.width: 14
                fillMode: Image.PreserveAspectFit
            }

        }

        VPNInterLabel {
            id: stabilityLabel
            lineHeight: Theme.controllerInterLineHeight
        }

    }

    Rectangle {
        Layout.preferredHeight: 4
        Layout.preferredWidth: 4
        color: "#FFFFFF"
        opacity: 0.8
        radius: 3
        Layout.leftMargin: Theme.windowMargin / 2
        Layout.rightMargin: Layout.leftMargin
    }

    VPNInterLabel {
        //% "Check Connection"
        text: qsTrId("vpn.connectionStability.checkConnection")
        color: "#FFFFFF"
        opacity: 0.8
        lineHeight: Theme.controllerInterLineHeight
    }

}
