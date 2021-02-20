/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RowLayout {
    property var numGridColumns: grid.columns
    onNumGridColumnsChanged: col.handleMultilineText();

    function setColumns() {
        grid.columns = grid.childrenRect.width > (window.width  - 32) ? 1 : 3;
        col.handleMultilineText();
    }
    id: stability

    Layout.preferredHeight: Theme.controllerInterLineHeight
    Layout.alignment: Qt.AlignHCenter
    spacing: 0
    opacity: 1
    state: VPNConnectionHealth.stability
    Component.onCompleted: labelActions.start()
    onStateChanged: labelActions.start()

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

        },
        State {
            name: VPNConnectionHealth.NoSignal
            extend: VPNConnectionHealth.Unstable
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
                id: stabilityLabel
                lineHeight: Theme.controllerInterLineHeight
                onPaintedHeightChanged: stability.setColumns();
            }
        }

        VPNStabilityLabelActions {
            id: labelActions
        }

        Rectangle {
            Layout.preferredHeight: 4
            Layout.preferredWidth: 4
            color: "#FFFFFF"
            opacity: 0.8
            radius: 3
            Layout.leftMargin: Theme.windowMargin / 2
            Layout.rightMargin: Layout.leftMargin
            visible: parent.columns === 3
        }

        VPNInterLabel {
            function setLineHeight() {
                lineHeight = grid.columns > 1 ? Theme.controllerInterLineHeight : 10
            }

            //% "Check Connection"
            //: Message displayed to the user when the connection is unstable or
            //: missing, asking them to check their connection.
            text: qsTrId("vpn.connectionStability.checkConnection")
            color: "#FFFFFF"
            opacity: 0.8
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignCenter
            onPaintedHeightChanged: {
                setLineHeight();
                stability.setColumns();
            }
            Component.onCompleted: setLineHeight()
        }
    }

}
