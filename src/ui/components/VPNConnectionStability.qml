/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15

Item {
    property var gridFlow: grid.flow
    id: stability

    function setColumns() {
        if (!visible) {
            return;
        }
        grid.flow = grid.children[0].width > (window.width - Theme.windowMargin * 2) ? Grid.TopToBottom : Grid.LeftToRight;
        col.handleMultilineText();
    }
    GridLayout {
        id: grid

        columnSpacing: 8
        columns: 3
        layoutDirection: Qt.LeftToRight
        anchors.horizontalCenter: parent.horizontalCenter
        state: VPNConnectionHealth.stability
        onStateChanged: stability.setColumns()

        Component.onCompleted: {
            flow = grid.children[0].width > window.width ? Grid.TopToBottom : Grid.LeftToRight
        }

        states: [
            State {
                name: VPNConnectionHealth.Stable
                PropertyChanges {
                    target: stability
                    visible: false
                    opacity: 0
                }
                PropertyChanges {
                    target: logoSubtitleOn
                    visible: true
                }
            },
            State {
                name: VPNConnectionHealth.Unstable

                PropertyChanges {
                    target: stability
                    visible: true
                    opacity: 1
                }
                PropertyChanges {
                    target: logoSubtitleOn
                    visible: false
                }
                PropertyChanges {
                    target: stabilityLabel
                    color: Theme.orange
                }
                PropertyChanges {
                    target: warningIcon
                    source: "../resources/warning-orange.svg"
                }
            },
            State {
                name: VPNConnectionHealth.NoSignal
                extend: VPNConnectionHealth.Unstable
                PropertyChanges {
                    target: stabilityLabel
                    color: Theme.red
                }
                PropertyChanges {
                    target: warningIcon
                    source: "../resources/warning.svg"
                }
            }
        ]

        Row {
            spacing: 6
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                height: 16
                width: 14
                color: "transparent"

                Image {
                    id: warningIcon

                    sourceSize.height: 14
                    sourceSize.width: 14
                    fillMode: Image.PreserveAspectFit
                }
            }

            VPNInterLabel {
                //% "Unstable"
                //: This refers to the user’s internet connection.
                readonly property var textUnstable: qsTrId(
                                                        "vpn.connectionStability.unstable")
                //% "No Signal"
                readonly property var textNoSignal: qsTrId(
                                                        "vpn.connectionStability.noSignal")

                id: stabilityLabel
                lineHeight: Theme.controllerInterLineHeight
                onPaintedWidthChanged: stability.setColumns()
                text: VPNConnectionHealth.stability
                      === VPNConnectionHealth.Unstable ? textUnstable : textNoSignal
                horizontalAlignment: Text.AlignLeft
            }
        }

        Rectangle {
            color: "#FFFFFF"
            opacity: 0.8
            radius: 3
            height: 4
            width: 4
            visible: grid.flow === Grid.LeftToRight
        }

        VPNInterLabel {
            //% "Check Connection"
            //: Message displayed to the user when the connection is unstable or
            //: missing, asking them to check their connection.
            text: qsTrId("vpn.connectionStability.checkConnection")
            color: "#FFFFFF"
            opacity: 0.8
            Layout.alignment: Qt.AlignCenter
            onPaintedWidthChanged: stability.setColumns()
            lineHeight: grid.flow === Grid.LeftToRight ? Theme.controllerInterLineHeight : 10
        }
    }

}
