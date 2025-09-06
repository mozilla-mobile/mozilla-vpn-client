/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

Item {
    id: box

    Layout.leftMargin: MZTheme.theme.listSpacing
    Layout.rightMargin: MZTheme.theme.listSpacing
    Layout.alignment: Qt.AlignHCenter
    states: [
        State {
            name: "stateInitializing"
            when: VPNController.state === VPNController.StateInitializing || VPNController.state === VPNController.StatePermissionRequired

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.bgColorStronger
            }

            PropertyChanges {
                target: logoTitle
                //% "VPN is off"
                text: qsTrId("vpn.controller.deactivated")
                color: MZTheme.colors.fontColorDark
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRings
                visible: false
            }

            PropertyChanges {
                target: connectionTime
                visible: false
            }

        },
        State {
            name: "stateOff"
            when: VPNController.state === VPNController.StateOff ||
                  VPNController.state === VPNController.StateOnPartial

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.bgColorStronger
            }

            PropertyChanges {
                target: logoTitle
                text: qsTrId("vpn.controller.deactivated")
                color: MZTheme.colors.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                text: qsTrId("vpn.controller.activationSloagan")
                color: MZTheme.colors.fontColor
                opacity: 1
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRings
                visible: false
            }

            PropertyChanges {
                target: connectionTime
                visible: false
            }

        },
        State {
            name: "stateConnecting"
            when: (VPNController.state === VPNController.StateConnecting)

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.primary
            }

            PropertyChanges {
                target: logoTitle
                //% "Connecting…"
                text: qsTrId("vpn.controller.connectingState")
                color: MZTheme.colors.fontColorInverted
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRings
                visible: false
            }

            PropertyChanges {
                target: connectionTime
                visible: false
            }

        },
        State {
            name: "stateConfirming"
            when: VPNController.state === VPNController.StateConfirming

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.primary
            }

            PropertyChanges {
                target: logoTitle
                text: qsTrId("vpn.controller.connectingState")
                color: MZTheme.colors.fontColorInverted
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRings
                visible: false
            }

            PropertyChanges {
                target: connectionTime
                visible: false
            }

        },
        State {
            name: "stateOn"
            when: (VPNController.state === VPNController.StateOn ||
                   VPNController.state === VPNController.StateSilentSwitching)

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.primary
            }

            PropertyChanges {
                target: logoTitle
                //% "VPN is on"
                text: qsTrId("vpn.controller.activated")
                color: MZTheme.colors.fontColorInverted
            }

            PropertyChanges {
                target: animatedRings
                visible: true
                opacity: 1
                startAnimation: true
            }

            PropertyChanges {
                target: connectionTime
                visible: true
            }

        },
        State {
            name: "stateDisconnecting"
            when: VPNController.state === VPNController.StateDisconnecting

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.bgColorStronger
            }

            PropertyChanges {
                target: logoTitle
                //% "Disconnecting…"
                text: qsTrId("vpn.controller.disconnecting")
                color: MZTheme.colors.fontColorDark
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRings
                visible: false
            }

            PropertyChanges {
                target: connectionTime
                visible: false
            }

        },
        State {
            name: "stateSwitching"
            when: VPNController.state === VPNController.StateSwitching

            PropertyChanges {
                target: boxBackground
                color: MZTheme.colors.primary
            }

            PropertyChanges {
                target: logoTitle
                //% "Switching…"
                text: qsTrId("vpn.controller.switching")
                color: MZTheme.colors.fontColorInverted
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRings
                visible: false
                opacity: 1
                startAnimation: false
            }

            PropertyChanges {
                target: connectionTime
                visible: false
            }

        }
    ]
    transitions: [
        Transition {
            to: "stateConnecting"

            ColorAnimation {
                target: boxBackground
                property: "color"
                duration: 200
            }

            ColorAnimation {
                target: logoTitle
                property: "color"
                duration: 200
            }

            ColorAnimation {
                target: logoSubtitle
                property: "color"
                duration: 200
            }

        },
        Transition {
            to: "stateDisconnecting"

            ColorAnimation {
                target: boxBackground
                property: "color"
                duration: 200
            }

            ColorAnimation {
                target: logoTitle
                property: "color"
                duration: 200
            }

            ColorAnimation {
                target: logoSubtitle
                property: "color"
                duration: 200
            }

        }
    ]

    function handleConnectionStateChange() {
        // Notify accessibility client of connection state
        if (!logoTitle.Accessible.ignored) {
            let notificationText = (logoTitle.text + '. ');
            if (logoSubtitle.visible)
                notificationText += (logoSubtitle.text + '. ');
            if (connectedStateDescription.visible)
                notificationText += (connectedStateDescription.text + '. ');

            MZAccessibleNotification.notify(logoTitle, notificationText);
        }
    }


    // Root Background Element
    Rectangle {
        id: boxBackground

        property var maximumBoxHeight: window.safeContentHeight - MZTheme.theme.windowMargin * 2 - (window.fullscreenRequired() ? box.y + MZTheme.theme.windowMargin : 0);
        color: MZTheme.colors.transparent

        radius: MZTheme.theme.cornerRadius * 2
        antialiasing: true

        height: box.height
        width: box.width
    }

    RowLayout {
        anchors.fill: box
        anchors.margins: MZTheme.theme.windowMargin
        spacing: 0
        ControllerImage {
                id: logo
                height: 48
                width: 48

        }
        Column {
            id: col
            spacing: 0
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            function handleMultilineText() {

                const titleIsWrapped = logoTitle.lineCount > 1;
                const subTitleIsWrapped = (logoSubtitle.visible && logoSubtitle.lineCount > 1 || (connectionStability.visible && connectionStability.gridFlow === Grid.TopToBottom));

                if (titleIsWrapped && subTitleIsWrapped) {
                    col.anchors.topMargin = 12
                    spacer.height = 6;
                    return;
                }

                if (subTitleIsWrapped) {
                    col.anchors.topMargin = 24
                    spacer.height = 4;
                    return;
                }

                if (titleIsWrapped) {
                    col.anchors.topMargin = 16
                    spacer.height = 12;
                    return;
                }
                col.anchors.topMargin = 24
                spacer.height = 16;
            }

            MZHeadline {
                id: logoTitle
                objectName: "controllerTitle"
                lineHeight: MZTheme.theme.labelLineHeight
                font.pixelSize: 16
                Accessible.ignored: !visible
 
                onPaintedHeightChanged: if (visible) col.handleMultilineText()
                onTextChanged: handleConnectionStateChange()
            }
            ConnectionTimer {
                id: connectionTime
                objectName: "connectionTimer"
                ignoreForAccessibility: true
            }

            ConnectionStability {
                id: connectionStability
                Accessible.ignored: !visible
                implicitHeight: childrenRect.height
            }
        }

        VPNToggle {
            id: toggle
            objectName: "controllerToggle"
            enabled: true
            Accessible.ignored: !visible
            Layout.preferredHeight: 32
            Layout.preferredWidth: 60
            scale: 0.8
        }
    }

    MZAnimatedRings {
        id: animatedRings
        // Make sure we only do the render animation when
        // The element is visible &&
        // the application is not minimized
        isCurrentyVisible: true
        // Magic numbers, i have no clue about qml positioning, sorry.
        // Anchors have failed me here.
        x: logo.x - 32.5
        y: logo.y

        width: 48 * 3
        height: 48 * 3

    
    }

}
