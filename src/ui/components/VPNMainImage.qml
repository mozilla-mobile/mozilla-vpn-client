/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Rectangle {
    id: logo

    property var showVPNOnIcon: false

    color: "transparent"
    opacity: 1
    onStateChanged: gradientGlobe.requestPaint()
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
                color: "#3FE1B0"
            }

            PropertyChanges {
                target: insetIcon
                source: "../resources/shield-on.svg"
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
                color: "#3FE1B0"
            }

            PropertyChanges {
                target: insetIcon
                source: "../resources/shield-on.svg"
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
                color: "#FF4F5E"
            }

            PropertyChanges {
                target: insetIcon
                source: "../resources/shield-off.svg"
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
                color: "#3FE1B0"
            }

        },
        State {
            name: VPNController.StateOff

            PropertyChanges {
                target: insetCircle
                color: "#FF4F5E"
            }

            PropertyChanges {
                target: insetIcon
                source: "../resources/shield-off.svg"
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
                color: "#3FE1B0"
            }

            PropertyChanges {
                target: insetIcon
                source: "../resources/shield-on.svg"
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
                color: "#FF4F5E"
            }

            PropertyChanges {
                target: insetIcon
                source: "../resources/shield-off.svg"
                opacity: 1
            }

        }
    ]
    transitions: [
        Transition {
            to: VPNController.StateConnecting
            ParallelAnimation {
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
                    duration: 500
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
            to: VPNController.StateOn
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
            source: "../resources/switching.svg"
            opacity: VPNController.state === VPNController.StateSwitching ? 1 : 0

             PropertyAnimation {
                 target: switchingIcon
                 property: "rotation"
                 from: 0
                 to: 360
                 loops: Animation.Infinite
                 duration: 8000
                 running: true
             }
        }
    }

    Canvas {
        id: gradientGlobe

        anchors.fill: logo
        contextType: "2d"
        antialiasing: true
        smooth: true
        onPaint: {
            if (!context) {
                return;
            }

            context.reset();
            // background fill
            let gradient = context.createRadialGradient(90, 0, 21, 90, 0, 100);
            if (!logo.showVPNOnIcon) {
                gradient.addColorStop(0, "#9D62FC");
                gradient.addColorStop(1, "#FD3296");
            } else {
                gradient.addColorStop(0, "#B833E1");
                gradient.addColorStop(0.25, "#9059FF");
                gradient.addColorStop(0.75, "#5B6DF8");
                gradient.addColorStop(1, "#0090ED");
            }
            // globe icon outline
            context.beginPath();
            context.path = "M38.4944 25.5505H27.8068C30.3243 17.2 34.8938 11.5 40 11.5C40.0882 11.5 40.1762 11.5017 40.264 11.5051C42.0749 7.93887 44.874 4.95904 48.3009 2.92608C45.5972 2.31825 42.8148 2.00316 40 2C32.4843 2 25.1374 4.22866 18.8883 8.40415C12.6393 12.5796 7.76872 18.5144 4.89259 25.458C2.01646 32.4016 1.26394 40.0421 2.73018 47.4134C4.19641 54.7847 7.81556 61.5557 13.13 66.87C18.4444 72.1844 25.2153 75.8036 32.5866 77.2698C39.9579 78.7361 47.5984 77.9835 54.542 75.1074C61.4856 72.2313 67.4204 67.3607 71.5958 61.1117C75.7713 54.8626 78 47.5157 78 40C77.9968 37.1852 77.6818 34.4028 77.0739 31.6991C75.0416 35.1249 72.063 37.9234 68.4982 39.7343C68.4992 39.8228 68.4998 39.9114 68.5 40C68.4933 43.1673 67.9555 46.3111 66.9088 49.3005H58.259C58.6472 46.8835 58.8834 44.4452 58.9663 42C57.3373 41.9974 55.7518 41.8094 54.2298 41.4559C54.157 44.0865 53.8876 46.7087 53.4235 49.3005H26.5765C25.4725 43.1493 25.4725 36.8507 26.5765 30.6995H40.3694C39.5317 29.0938 38.8955 27.3663 38.4944 25.5505ZM27.8068 54.4495C30.3243 62.8 34.8938 68.5 40 68.5C45.1063 68.5 49.6758 62.8 52.198 54.4495H27.8068ZM13.0913 30.6995C12.0446 33.6889 11.5067 36.8327 11.5 40C11.5067 43.1673 12.0446 46.3111 13.0913 49.3005H21.741C20.7526 43.1395 20.7526 36.8604 21.741 30.6995H13.0913ZM22.8478 25.5505C23.8606 21.5329 25.5487 17.7166 27.84 14.2645C22.6866 16.7113 18.3813 20.6411 15.4758 25.5505H22.8478ZM22.8478 54.4495H15.49H15.4758C18.3813 59.3588 22.6866 63.2887 27.84 65.7355C25.5487 62.2834 23.8606 58.4671 22.8478 54.4495ZM52.1808 65.7256C57.3302 63.2805 61.6331 59.3544 64.5385 54.4495H57.1665C56.1547 58.4633 54.4688 62.2761 52.1808 65.7256Z";
            context.closePath();
            // globe icon fill rules
            context.fillStyle = gradient;
            context.clip("evenodd");
            context.fill("evenodd");
        }
    }

}
