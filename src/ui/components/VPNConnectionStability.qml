/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import generated 0.15

RowLayout {
    property var numGridColumns: grid.columns

    function setColumns() {
        grid.columns = grid.childrenRect.width > (window.width  - 48) ? 1 : 3;
        col.handleMultilineText();
    }
    id: stability

    Layout.preferredHeight: Theme.controllerInterLineHeight
    Layout.alignment: Qt.AlignHCenter
    spacing: 0
    state: VPNConnectionHealth.stability
    onStateChanged: setColumns();

    states: [
        State {
            name: VPNConnectionHealth.Stable
            PropertyChanges {
                target: stability
                visible: false
                opacity: 0
            }
            PropertyChanges {
                target: logoSubtitle
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
                target: logoSubtitle
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

    GridLayout {
        id: grid
        columnSpacing: 0
        state: parent.state
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Component.onCompleted: setColumns()

        RowLayout {
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
                readonly property var textUnstable: qsTrId("vpn.connectionStability.unstable")
                //% "No Signal"
                readonly property var textNoSignal: qsTrId("vpn.connectionStability.noSignal")


                id: stabilityLabel

                lineHeight: Theme.controllerInterLineHeight
                onPaintedWidthChanged: setColumns();
                text: VPNConnectionHealth.stability === VPNConnectionHealth.Unstable ? textUnstable : textNoSignal
            }
        }

        Rectangle {
            color: "#FFFFFF"
            opacity: 0.8
            radius: 3
            Layout.leftMargin: Theme.windowMargin / 2
            Layout.rightMargin: Layout.leftMargin
            Layout.preferredHeight: 4
            Layout.preferredWidth: 4
            visible: parent.columns === 3
        }

        VPNInterLabel {
            //% "Check Connection"
            //: Message displayed to the user when the connection is unstable or
            //: missing, asking them to check their connection.
            text: qsTrId("vpn.connectionStability.checkConnection")
            color: "#FFFFFF"
            opacity: 0.8
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignCenter
            onPaintedWidthChanged: setColumns();
            lineHeight: grid.columns > 1 ? Theme.controllerInterLineHeight : 10
        }
    }

}
