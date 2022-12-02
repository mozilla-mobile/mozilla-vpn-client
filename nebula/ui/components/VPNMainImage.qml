/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

Rectangle {
    id: logo

    property var showVPNOnIcon: false

    color: VPNTheme.theme.transparent
    opacity: 1
    states: [
        State {
            name: "stateConnecting"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateConnecting

            PropertyChanges {
                target: logo
                opacity: 0.6
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.success.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-on.svg"
                opacity: 1
            }
        },
        State {
            name: "stateConfirming"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateConfirming

            PropertyChanges {
                target: logo
                opacity: 0.6
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.success.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-on.svg"
                opacity: 1
            }

        },
        State {
            name: "stateDisconnecting"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateDisconnecting

            PropertyChanges {
                target: logo
                opacity: 0.55
                showVPNOnIcon: false
            }

            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.error.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-off.svg"
                opacity: 1
            }

        },
        State {
            name: "stateSwitching"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateSwitching

            PropertyChanges {
                target: logo
                opacity: 0.55
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.success.default
            }
            PropertyChanges {
                target: switchingIcon
                opacity: 1
            }
            PropertyChanges {
                target: spin
                running: true
            }
        },
        State {
            name: "stateOff"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateOff

            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.error.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-off.svg"
                opacity: 1
            }

        },
        State {
            name: "stateInitializing"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateInitializing

            PropertyChanges {
                target: logo
                showVPNOnIcon: false
                opacity: 0.55
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.success.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-off.svg"
                opacity: 1
            }
        },
        State {
            name: "stateOn"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateOn &&
                MZModules["vpn"].connectionHealth.stability === MZModules["vpn"].connectionHealth.Stable

            PropertyChanges {
                target: logo
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.success.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-on.svg"
                opacity: 1
            }
        },
        State {
            name: "unstableOn"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateOn &&
                MZModules["vpn"].connectionHealth.stability === MZModules["vpn"].connectionHealth.Unstable

            PropertyChanges {
                target: logo
                showVPNOnIcon: true
                opacity: 1
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.warning.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-on.svg"
                opacity: 1
            }
        },
        State {
            name: "noSignalOn"
            when: MZModules["vpn"].controller.state === MZModules["vpn"].controller.StateOn &&
                MZModules["vpn"].connectionHealth.stability === MZModules["vpn"].connectionHealth.NoSignal

            PropertyChanges {
                target: logo
                showVPNOnIcon: true
                opacity: 1
            }
            PropertyChanges {
                target: insetCircle
                color: VPNTheme.colors.error.default
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/nebula/resources/shield-off.svg"
                opacity: 1
            }
        }
    ]
    transitions: [
        Transition {
            to: MZModules["vpn"].controller.StateConnecting
            ParallelAnimation {
                PropertyAnimation {
                    target: insetCircle
                    property: "color"
                    duration: 100
                }

                PropertyAnimation {
                    target: logo
                    property: "opacity"
                    duration: 200
                }

                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 0.9
                    duration: 200
                    easing.type: Easing.Linear
                }
            }
        },
        Transition {
            to: MZModules["vpn"].controller.StateConfirming
            ParallelAnimation {

                PropertyAnimation {
                    target: insetIcon
                    property: "opacity"
                    duration: 100
                }
            }

        },
        Transition {
            to: MZModules["vpn"].controller.StateDisconnecting
            ParallelAnimation {
                PropertyAnimation {
                    target: insetCircle
                    property: "color"
                    duration: 100
                }
                PropertyAnimation {
                    target: logo
                    property: "opacity"
                    duration: 200
                }

                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 0.9
                    duration: 150
                    easing.type: Easing.Linear
                }
            }
        },
        Transition {
            to: MZModules["vpn"].controller.StateOff

            ParallelAnimation {
                PropertyAnimation {
                    target: logo
                    property: "opacity"
                    duration: 100
                }

                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 1
                    duration: 100
                    easing.type: Easing.InOutBounce
                }
            }
        },
        Transition {
            to: MZModules["vpn"].controller.StateSwitching
            ParallelAnimation {
                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 0.9
                    duration: 200
                    easing.type: Easing.Linear
                }

                PropertyAnimation {
                    target: logo
                    property: "opacity"
                    duration: 300
                }

                PropertyAnimation {
                    target: insetIcon
                    property: "opacity"
                    to: 0
                    duration: 50
                }
            }
        },
        Transition {
            to: "*"
            ParallelAnimation {
                PropertyAnimation {
                    target: insetIcon
                    property: "opacity"
                    to: 1
                    duration: 200
                }
                PropertyAnimation {
                    target: logo
                    property: "opacity"
                    duration: 300
                }
                PropertyAnimation {
                    target: insetCircle
                    property: "color"
                    duration: 100
                }
                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 1
                    duration: 150
                    easing.type: Easing.InOutBounce
                }
            }
        }
    ]

    Rectangle {
        // green or red circle in upper right hand area of
        // gradientGlobe
        id: insetCircle

        height: 32
        width: 32
        radius: 16
        x: 43
        y: 5
        antialiasing: true
        smooth: true

        Image {
            id: insetIcon

            sourceSize.height: 32
            sourceSize.width: 32
            anchors.centerIn: insetCircle
            opacity: 1
        }

        Image {
            id: switchingIcon
            anchors.centerIn: insetCircle
            sourceSize.height: 32
            sourceSize.width: 32
            source: "qrc:/nebula/resources/switching.svg"
            opacity: 0
             PropertyAnimation {
                 id: spin
                 target: switchingIcon
                 property: "rotation"
                 from: 0
                 to: 360
                 loops: Animation.Infinite
                 duration: 8000
                 running: false
             }
        }
    }

    Image {
        id: globe
        source: logo.showVPNOnIcon ? "qrc:/ui/resources/main-img-vpn-on.svg" : "qrc:/ui/resources/main-img-vpn-off.svg"
        sourceSize.height: logo.height
        sourceSize.width: logo.width
        visible: true
    }
}
