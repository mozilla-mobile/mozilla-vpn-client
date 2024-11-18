/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

Rectangle {
    id: logo

    property bool showVPNOnIcon: false

    color: MZTheme.colors.transparent
    opacity: 1
    state: VPNController.state
    states: [
        State {
            name: VPNController.StateConnecting
            PropertyChanges {
                target: logo
                opacity: 0.6
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.successAccent
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-on.svg"
                opacity: 1
            }
        },
        State {
            name: VPNController.StateConfirming

            PropertyChanges {
                target: logo
                opacity: 0.6
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.successAccent
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-on.svg"
                opacity: 1
            }

        },
        State {
            name: VPNController.StateDisconnecting

            PropertyChanges {
                target: logo
                opacity: 0.55
                showVPNOnIcon: false
            }

            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.errorAccentLight
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-off.svg"
                opacity: 1
            }
        },
        State {
            name: VPNController.StateSwitching

            PropertyChanges {
                target: logo
                opacity: 0.55
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.successAccent
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
            name: VPNController.StateOff
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.errorAccentLight
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-off.svg"
                opacity: 1
            }
        },
        State {
            name: VPNController.StateOnPartial
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.errorAccentLight
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-off.svg"
                opacity: 1
            }

        },
        State {
            name: VPNController.StateInitializing

            PropertyChanges {
                target: logo
                showVPNOnIcon: false
                opacity: 0.55
            }
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.successAccent
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-off.svg"
                opacity: 1
            }
        },
        State {
            name: VPNController.StateOn
            PropertyChanges {
                target: logo
                showVPNOnIcon: true
            }
            PropertyChanges {
                target: insetCircle
                color: MZTheme.colors.successAccent
            }
            PropertyChanges {
                target: insetIcon
                source: "qrc:/ui/resources/shield-on.svg"
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
                    duration: 100
                }

                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 0.9
                    duration: 100
                    easing.type: Easing.Linear
                }
            }
        },
        Transition {
            to: VPNController.StateConfirming
            ParallelAnimation {
                PropertyAnimation {
                    target: insetCircle
                    property: "color"
                    duration: 100
                }

                PropertyAnimation {
                    target: logo
                    property: "opacity"
                    duration: 100
                }

                PropertyAnimation {
                    target: insetCircle
                    property: "scale"
                    to: 0.9
                    duration: 100
                    easing.type: Easing.Linear
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
                    to: 0.85
                    duration: 200
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
                    duration: 100
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


    Connections{
        target: VPNConnectionHealth
        function onStabilityChanged() {
            if (VPNController.state === VPNController.StateOnPartial) {
                return;
            }
            switch(VPNConnectionHealth.stability) {
                case(VPNConnectionHealth.NoSignal):
                    insetCircle.color = MZTheme.colors.errorAccentLight;
                    insetIcon.source = "qrc:/ui/resources/shield-off.svg";
                    break;
            
                case(VPNConnectionHealth.Stable):
                    insetCircle.color = MZTheme.colors.successAccent;
                    insetIcon.source = "qrc:/ui/resources/shield-on.svg";
                    break;
            
                case(VPNConnectionHealth.Unstable):
                    insetCircle.color = MZTheme.colors.warningAccent;
                    break;
            
            }
        }
    }

    Rectangle {
        // green or red circle in upper right hand area of
        // gradientGlobe
        id: insetCircle

        height: 32
        width: 32
        radius: 16
        x: 44
        y: 4
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
            source: "qrc:/ui/resources/switching.svg"
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
