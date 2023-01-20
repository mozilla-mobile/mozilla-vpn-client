/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

import org.mozilla.Glean 0.30
import telemetry 0.30

Item {
    property real gridFlow: grid.flow
    id: stability

    function setColumns() {
        if (!visible) {
            return;
        }
        grid.flow = grid.children[0].width > (window.width - VPNTheme.theme.windowMargin * 2) ? Grid.TopToBottom : Grid.LeftToRight;
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
                    visible: VPNController.state === VPNController.StateOn
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
                    color: VPNTheme.theme.orange
                }
                PropertyChanges {
                    target: warningIcon
                    source: "qrc:/nebula/resources/warning-orange.svg"
                }
            },
            State {
                name: VPNConnectionHealth.NoSignal
                extend: VPNConnectionHealth.Unstable
                PropertyChanges {
                    target: stabilityLabel
                    color: VPNTheme.theme.red
                }
                PropertyChanges {
                    target: warningIcon
                    source: "qrc:/nebula/resources/warning.svg"
                }
                PropertyChanges {
                    target: logoSubtitleOn
                    visible: false
                }
            }
        ]

        Row {
            spacing: 6
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                height: 16
                width: VPNTheme.theme.iconSizeSmall
                color: VPNTheme.theme.transparent

                Image {
                    id: warningIcon

                    sourceSize.height: VPNTheme.theme.iconSizeSmall
                    sourceSize.width: VPNTheme.theme.iconSizeSmall
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
                lineHeight: VPNTheme.theme.controllerInterLineHeight
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
            lineHeight: grid.flow === Grid.LeftToRight ? VPNTheme.theme.controllerInterLineHeight : 10
        }
    }

}
