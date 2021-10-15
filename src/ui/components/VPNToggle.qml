/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.5

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

VPNButtonBase {
    id: toggleButton

    property var connectionRetryOverX: VPNController.connectionRetry > 1
    property var toggleColor: Theme.vpnToggleDisconnected
    property var toolTipTitle: ""
    Accessible.name: toolTipTitle

    function handleClick() {
        toolTip.close();
        if (VPNController.state !== VPNController.StateOff) {
            return VPN.deactivate();
        }

        return VPN.activate();
    }

    onClicked: handleClick()

    // property in VPNButtonBase {}
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
                color: "#9E9E9E"
                border.color: Theme.white
            }

            PropertyChanges {
                target: toggleButton
                toggleColor: Theme.vpnToggleDisconnected
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
                color: "#9E9E9E"
                border.color: Theme.white
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN on"
                toolTipTitle: qsTrId("vpn.toggle.on")
            }

        },
        State {
            name: VPNController.StateConnecting

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: "#998DB2"
            }

            PropertyChanges {
                target: toggle
                color: "#387E8A"
                border.color: Theme.ink
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN off"
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: Theme.vpnToggleConnected
            }

        },
        State {
            name: VPNController.StateConfirming

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: connectionRetryOverX ? "#FFFFFF" : "#998DB2"
            }

            PropertyChanges {
                target: toggle
                color: "#387E8A"
                border.color: Theme.ink
            }

            PropertyChanges {
                target: toggleButton
                //% "Turn VPN off"
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: Theme.vpnToggleConnected
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
                color: "#3FE1B0"
                border.color: Theme.ink
            }

            PropertyChanges {
                target: toggleButton
                toolTipTitle: qsTrId("vpn.toggle.off")
                toggleColor: Theme.vpnToggleConnected
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
                color: "#CECECE"
                border.color: Theme.white
            }

            PropertyChanges {
                target: toggleButton
                toolTipTitle: qsTrId("vpn.toggle.on")
                toggleColor: Theme.vpnToggleDisconnected
            }

        },
        State {
            name: VPNController.StateSwitching

            PropertyChanges {
                target: cursor
                anchors.leftMargin: 32
                color: "#998DB2"
            }

            PropertyChanges {
                target: toggle
                color: "#387E8A"
                border.color: Theme.ink
            }

            PropertyChanges {
                target: toggleButton
                toggleColor: Theme.vpnToggleConnected
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
    VPNFocusBorder {
        id: focusHandler

        anchors.fill: toggle
        anchors.margins: -4
        radius: height / 2
        border.color: toggleColor.focusBorder
        color: "transparent"
        opacity: toggleButton.activeFocus && (VPNController.state === VPNController.StateOn || VPNController.state === VPNController.StateOff) ? 1 : 0

        VPNFocusOutline {
            id: vpnFocusOutline

            anchors.fill: focusHandler
            focusedComponent: focusHandler
            setMargins: -6
            radius: height / 2
            border.width: 7
            color: "transparent"
            border.color: toggleColor.focusOutline
            opacity: 0.25
        }

    }

    // Faint outline visible on hover and press
    Rectangle {
        id: hoverPressHandler

        color: "#C2C2C2"
        state: toggle.state
        opacity: {
            if (state === uiState.stateDefault || toggleButton.activeFocus)
                return 0;

            if (state === uiState.stateHovered)
                return 0.2;

            if (state === uiState.statePressed)
                return 0.3;

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
                VPNController.state === VPNController.StateOff ||
                (VPNController.state === VPNController.StateConfirming && connectionRetryOverX));
    }

    // Toggle background color changes on hover and press
    VPNUIStates {
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

    VPNMouseArea {
        id: mouseArea

        targetEl: toggle
        anchors.fill: toggle
        hoverEnabled: toggleClickable()
        cursorShape: Qt.PointingHandCursor
    }

    VPNToolTip {
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
                duration: 200
            }

        }

    }

}

