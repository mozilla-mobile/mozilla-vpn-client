/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZButtonBase {
    id: toggleButton

    property var connectionRetryOverX: VPNController.connectionRetry > 1
    property var enableDisconnectInConfirming: VPNController.enableDisconnectInConfirming
    property var toggleColor: {}
    property var toolTipTitle: ""
    Accessible.name: toolTipTitle

    function handleClick() {
        toolTip.close();
        if (VPNController.state !== VPNController.StateOff &&
            VPNController.state !== VPNController.StateOnPartial) {
            return VPN.deactivate();
        }

        return VPN.activate();
    }

    onClicked: handleClick()

    // property in MZButtonBase {}
    visualStateItem: toggle
    state: VPNController.state
    height: 32
    width: 60
    radius: 16
    hoverEnabled: false

    onActiveFocusChanged: {
        if (!focus && toolTip.visible) {
            toolTip.close();
        }
    }

    states: [
        State {
            name: VPNController.StateInitializing

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.grey30
                border.color: MZTheme.colors.white
            }

            PropertyChanges {
                target: toggleButton
                toggleColor: MZTheme.colors.vpnToggleDisconnected
            }

        },
        State {
            name: VPNController.StateOff

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.grey30
                border.color: MZTheme.colors.white
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN on"
                toolTipTitle: qsTrId("vpn.toggle.on")
                toggleColor: MZTheme.colors.vpnToggleDisconnected
            }

        },

        State {
            name: VPNController.StateOnPartial
            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.grey30
                border.color: MZTheme.colors.white
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN on"
                toolTipTitle: qsTrId("vpn.toggle.on")
                toggleColor: MZTheme.colors.vpnToggleDisconnected
            }
        },


        State {
            name: VPNController.StateConnecting

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: MZTheme.colors.dullPurple
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.dullGreen
                border.color: MZTheme.colors.ink
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN off"
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: MZTheme.colors.vpnToggleConnected
            }

        },
        State {
            name: VPNController.StateConfirming

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: connectionRetryOverX ? MZTheme.colors.white : MZTheme.colors.dullPurple
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.dullGreen
                border.color: MZTheme.colors.ink
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN off"
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: MZTheme.colors.vpnToggleConnected
            }

        },
        State {
            name: VPNController.StateOn

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.green50
                border.color: MZTheme.colors.ink
            }

            PropertyChanges {
                target: toggleButton
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: MZTheme.colors.vpnToggleConnected
            }
        },

        State {
            name: VPNController.StateSilentSwitching

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.green50
                border.color: MZTheme.colors.ink
            }

            PropertyChanges {
                target: toggleButton
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: MZTheme.colors.vpnToggleConnected
            }
        },
        State {
            name: VPNController.StateDisconnecting

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 4
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.grey20
                border.color: MZTheme.colors.white
            }

            PropertyChanges {
                target: toggleButton
                toolTipTitle: qsTrId("vpn.toggle.on")
                toggleColor: MZTheme.colors.vpnToggleDisconnected
            }

        },
        State {
            name: VPNController.StateSwitching

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: MZTheme.colors.dullPurple
            }

            PropertyChanges {
                target: toggle
                color: MZTheme.colors.dullGreen
                border.color: MZTheme.colors.ink
            }

            PropertyChanges {
                target: toggleButton
                toggleColor: MZTheme.colors.vpnToggleConnected
            }

        }
    ]
    transitions: [
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    target: cursor
                    property: "anchors.leftMargin"
                    duration: 200
                }

                ColorAnimation {
                    target: cursor
                    duration: 200
                }

            }

        }
    ]

    // Focus rings
    MZFocusBorder {
        id: focusHandler

        anchors.fill: toggle
        anchors.margins: -4
        radius: height / 2
        border.color: toggleColor.focusBorder
        color: MZTheme.colors.transparent
        opacity: toggleButton.activeFocus && (VPNController.state === VPNController.StateOn || VPNController.state === VPNController.StateOnPartial || VPNController.state === VPNController.StateSilentSwitching || VPNController.state === VPNController.StateOff) ? 1 : 0

        MZFocusOutline {
            id: vpnFocusOutline

            anchors.fill: focusHandler
            focusedComponent: focusHandler
            setMargins: -6
            radius: height / 2
            border.width: 7
            color: MZTheme.colors.transparent
            border.color: toggleColor.focusOutline
            opacity: 0.25
        }

    }

    // Faint outline visible on hover and press
    Rectangle {
        id: hoverPressHandler

        color: MZTheme.colors.grey20
        state: toggle.state
        opacity: {
            if (state === uiState.stateHovered)
                return 0.2;

            if (state === uiState.statePressed)
                return 0.3;

            return 0;
        }
        z: -1
        anchors.fill: toggle
        radius: height / 2
        anchors.margins: -5

        PropertyAnimation on opacity {
            duration: 200
        }

    }

    function toggleClickable() {
        return VPN.state === VPN.StateMain &&
               (VPNController.state === VPNController.StateOn ||
               VPNController.state === VPNController.StateOnPartial ||
                VPNController.state === VPNController.StateSilentSwitching ||
                VPNController.state === VPNController.StateOff ||
                (VPNController.state === VPNController.StateConfirming &&
                 (connectionRetryOverX || enableDisconnectInConfirming)));
    }

    // Toggle background color changes on hover and press
    MZUIStates {
        itemToFocus: toggleButton
        itemToAnchor: toggle
        colorScheme: toggleColor
        radius: height / 2
        setMargins: -7
        showFocusRings: false
        opacity: toggleClickable() ? 1 : 0
        z: 1

        Behavior on opacity {
            PropertyAnimation {
                property: "opacity"
                duration: 100
            }
        }
    }

    Rectangle {
        id: cursor

        height: 24
        width: 24
        radius: 12
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 4
        z: 2
    }

    MZMouseArea {
        id: mouseArea

        targetEl: toggle
        anchors.fill: toggle
        hoverEnabled: toggleButton.enabled && toggleClickable()
        cursorShape: Qt.PointingHandCursor
    }

    MZToolTip {
        id: toolTip
        text: toolTipTitle
    }

    background: Rectangle {
        id: toggle

        Component.onCompleted: state = uiState.stateDefault
        border.width: 0
        anchors.fill: toggleButton
        radius: height / 2

        Behavior on color {
            ColorAnimation {
                duration: 100
            }

        }

    }

}

