/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

Item {
    property real gridFlow: grid.flow
    id: stability

    //% "Check Connection"
    //: Message displayed to the user when the connection is unstable or
    //: missing, asking them to check their connection.
    readonly property var textCheckConnection: qsTrId("vpn.connectionStability.checkConnection")

    function setColumns() {
        if (!visible) {
            return;
        }
        grid.flow = grid.children[0].width > (window.width - MZTheme.theme.windowMargin * 2) ? Grid.TopToBottom : Grid.LeftToRight;
        col.handleMultilineText();
    }

    function handleConnectionHealthStateChange() {
        // If the stability item is not visible, there are no connection health problems to be be processed
        if (!visible) {
            return;
        }

        // Notify accessibility client of connection health problems
        if (!stabilityLabel.Accessible.ignored)
        {
            let notificationText = stabilityLabel.text + ". " + stabilityLabelInstruction.text;
            MZAccessibleNotification.notify(stabilityLabel, notificationText);
        }
    }

    function stabilityLabelText() {
        if (VPNController.state == VPNController.StateConnectionError && (VPNController.error == VPNController.ErrorServerTimeout || VPNController.error == VPNController.ErrorNoServerAvailable)) {
            // TODO maybe use a different string than ModalHeaderText here
            return MZI18n.ServerUnavailableModalHeaderText;
        }
        return textCheckConnection;
    }

    GridLayout {
        id: grid

        columnSpacing: 8
        columns: 3
        layoutDirection: Qt.LeftToRight
        anchors.horizontalCenter: parent.horizontalCenter
        state: VPNController.state != VPNController.StateConnectionError ? VPNConnectionHealth.stability : VPNConnectionHealth.NoSignal
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
                    visible: (VPNController.state !== VPNController.StateOnPartial)
                    opacity: 1
                }
                PropertyChanges {
                    target: logoSubtitleOn
                    visible: false
                }
                PropertyChanges {
                    target: stabilityLabel
                    color: MZTheme.colors.fontColorWarningForConnectionBox
                }
                PropertyChanges {
                    target: warningIcon
                    source: MZAssetLookup.getImageSource("WarningOrange")
                }
                StateChangeScript { script: handleConnectionHealthStateChange(); }
            },
            State {
                name: VPNConnectionHealth.NoSignal
                extend: VPNConnectionHealth.Unstable
                PropertyChanges {
                    target: stability
                    visible: (VPNController.state !== VPNController.StateOnPartial)
                }
                PropertyChanges {
                    target: stabilityLabel
                    color: MZTheme.colors.errorAccentLight
                }
                PropertyChanges {
                    target: warningIcon
                    source: MZAssetLookup.getImageSource("WarningRed")
                }
                PropertyChanges {
                    target: logoSubtitleOn
                    visible: false
                }
                StateChangeScript { script: handleConnectionHealthStateChange(); }
            }
        ]

        Row {
            spacing: 6
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                height: 16
                width: MZTheme.theme.iconSizeSmall
                color: MZTheme.colors.transparent

                Image {
                    id: warningIcon

                    sourceSize.height: MZTheme.theme.iconSizeSmall
                    sourceSize.width: MZTheme.theme.iconSizeSmall
                    fillMode: Image.PreserveAspectFit
                }
            }

            MZInterLabel {
                //% "Unstable"
                //: This refers to the user’s internet connection.
                readonly property var textUnstable: qsTrId(
                                                        "vpn.connectionStability.unstable")
                //% "No Signal"
                readonly property var textNoSignal: qsTrId(
                                                        "vpn.connectionStability.noSignal")

                id: stabilityLabel
                objectName: "stabilityLabel"
                color: MZTheme.colors.fontColorDark
                lineHeight: MZTheme.theme.controllerInterLineHeight
                onPaintedWidthChanged: stability.setColumns()
                text: VPNConnectionHealth.stability
                      === VPNConnectionHealth.Unstable ? textUnstable : textNoSignal
                horizontalAlignment: Text.AlignLeft
            }
        }

        Rectangle {
            color: MZTheme.colors.fontColorInverted
            opacity: 0.8
            radius: 3
            height: 4
            width: 4
            visible: grid.flow === Grid.LeftToRight
        }

        MZInterLabel {
            id: stabilityLabelInstruction
            objectName: "stabilityLabelInstruction"
            text: stabilityLabelText()
            color: MZTheme.colors.fontColorInverted
            opacity: 0.8
            Layout.alignment: Qt.AlignCenter
            onPaintedWidthChanged: stability.setColumns()
            lineHeight: grid.flow === Grid.LeftToRight ? MZTheme.theme.controllerInterLineHeight : 10
        }
    }

}
