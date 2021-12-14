/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import compat 0.1

Rectangle {
    id: logo

    property var showVPNOnIcon: false

    color: "transparent"
    opacity: 1
    states: [
        State {
            name: "stateConnecting"
            when: VPNController.state === VPNController.StateConnecting

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
            when: VPNController.state === VPNController.StateConfirming

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
            when: VPNController.state === VPNController.StateDisconnecting

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
            when: VPNController.state === VPNController.StateSwitching

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
            when: VPNController.state === VPNController.StateOff

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
            when: VPNController.state === VPNController.StateInitializing

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
            when: VPNController.state === VPNController.StateOn &&
                VPNConnectionHealth.stability === VPNConnectionHealth.Stable

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
            when: VPNController.state === VPNController.StateOn &&
                VPNConnectionHealth.stability === VPNConnectionHealth.Unstable

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
            when: VPNController.state === VPNController.StateOn &&
                VPNConnectionHealth.stability === VPNConnectionHealth.NoSignal

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
            to: VPNController.StateConnecting
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
            to: VPNController.StateConfirming
            ParallelAnimation {

                PropertyAnimation {
                    target: insetIcon
                    property: "opacity"
                    duration: 100
                }
            }

        },
        Transition {
            to: VPNController.StateDisconnecting
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
            to: VPNController.StateOff

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
            to: VPNController.StateSwitching
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
        source: "qrc:/nebula/resources/globe.svg";
        sourceSize.height: logo.height
        sourceSize.width: logo.width
        visible: false
    }

    VPNLinearGradient {
        id: gradient
        anchors.fill: logo
        start: Qt.point(0, logo.width)
        end: Qt.point(logo.height,0)
        gradient:
            Gradient {
                  GradientStop {
                      id: stop1
                      position: 0
                      color: logo.showVPNOnIcon ? "#0090ED": "#FD3296"
                  }
                  GradientStop {
                      id: stop2
                      position: 1
                      color: logo.showVPNOnIcon ? "#B833E1" : "#9D62FC"
                  }
              }
        visible: false
    }

    VPNOpacityMask {
       anchors.fill: globe
       source: gradient
       maskSource: globe
       cached: true
   }
}
